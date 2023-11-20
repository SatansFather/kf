import os
from enum import Enum

HashResults = {}

class ClassInfo:

	class Property:
		Type = ""
		Name = ""
		DefaultValue = ""
		ConditionFunction = ""
		FirstOnly = False
		ViewOnly = False
		ReplayOnly = False
		SkipReplay = False
		OwnerOnly = False
		OthersOnly = False
		Transient = False
		Destroy = False
		OnRepFunction = ""

	ParentNames = []
	ClassName = ""
	DestroyName = ""
	HeaderPath = ""
	Hash = ""
	IsSnapshottable = False
	SnapshotParent = None
	Properties = []
	OnRepFunctions = []

	def __init__(self):
		self.ParentNames = []
		self.ClassName = ""
		self.DestroyName = ""
		self.HeaderPath = ""
		self.Hash = 0
		self.Properties = []

	def HashName(self, addCount):
		global HashResults
		self.Hash = 0
		byteName = str.encode(self.ClassName)
		for char in byteName:
			self.Hash = self.Hash + char
		self.Hash += addCount
		self.Hash = str(self.Hash % 256)

		if self.Hash in HashResults:
			print("WARNING: " + self.ClassName + " hash match with " + HashResults[self.Hash])
			print("increasing value by 1...")
			self.HashName(addCount + 1)
			return

		HashResults[self.Hash] = self.ClassName

	def GetStructName(self):
		return "KSnapshot_" + self.ClassName
		
# stores ClassInfo
classData = []

instantiableClasses = ['KNetPlayer']
with open ("src/engine/create_pools.cpp", 'r') as file:
	for l in file:
		line = l.strip();
		if line.startswith("AddPool"):
			className = line.split('<')[1].split('>')[0].strip();
			instantiableClasses.append(className)

# headers that might contain net info
potentialHeaders = ["src/engine/net/player.h"]

# iterate entity headers to find snapshottables
for currentDir, subDirs, files in os.walk("src/game/entity"):
	for file in files:
		if (file.endswith(".h")):
			potentialHeaders.append(currentDir.replace('\\', '/') + '/' + file)

# look for snapshottables in headers
for header in potentialHeaders:
	with open (header, 'r') as file:

		classInfo = ClassInfo()
		insideClass = False
		readingMembers = False
		insideString = False
		scopeDepth = 0
		lastWasEscapeChar = False
		parentClassBuffer = ""

		classScopeDepth = 0

		for l in file:
			line = l.strip()

			for char in line:
				if insideString:
					if char == '\\' and not lastWasEscapeChar:
						lastWasEscapeChar = True
						continue
				elif char == '{':
					scopeDepth += 1
				elif char == '}':
					scopeDepth -= 1
				
				if char == '"' and not lastWasEscapeChar:
					insideString = not insideString

				lastWasEscapeChar = False

			assert not insideString, "inside string at end of line"



			if not insideClass:
				if line.strip().startswith("class K") and not line.endswith(';'):
					classScopeDepth = scopeDepth
					classInfo.ClassName = line.split("class", 1)[1].split(":")[0].strip()
					classInfo.HeaderPath = header.split("src/")[1]
					insideClass = True
			
			if not insideClass:
				continue
			
			# we are inside a class at this point

			if scopeDepth == classScopeDepth:
				if readingMembers:
					# we have left the class
					# add info and reset in case this file has more classes

					classData.append(classInfo)
	
					classInfo = ClassInfo()
					insideClass = False
					readingMembers = False
					parentClassBuffer = ""

				else:
					# should be reading parent classes
					parentClassBuffer += line					
					
			elif scopeDepth > classScopeDepth:
				# parse parent classes
				# TODO multi arg templates probably break this
				if parentClassBuffer:

					#tempStart = parentClassBuffer.find('<')
					#tempEnd = parentClassBuffer.find('>')
					#if tempStart != -1 and tempEnd != -1:
					#	parentClassBuffer = parentClassBuffer[tempStart : tempEnd : 1]
					#print(classInfo.ClassName)
					#print(parentClassBuffer)
						

					parentClassBuffer = parentClassBuffer.replace('\n', ' ').strip()
					if ':' in parentClassBuffer:
						parentClassBuffer = parentClassBuffer.split(':')[1] 
					parents = parentClassBuffer.split(',')
					for p in parents:
						p = p.replace("public ", "")
						p = p.replace("private ", "")
						p = p.replace("virtual ", "")
						p = p.replace("#if !_SERVER", "")
						p = p.strip()
						classInfo.ParentNames.append(p)
					parentClassBuffer = ""

				readingMembers = True

				# looking through class members now

				if line.startswith("void OnRep_"):
					# parse OnRep function
					func = line.split("OnRep_")[1].split("(")[0].strip()
					classInfo.OnRepFunctions.append(func)

				# try to find net macros
				if line.startswith("SNAP_PROP"):
					prop = ClassInfo.Property()
					splitStr = "SNAP_PROP("
					if line.startswith("SNAP_PROP_TRANSIENT"):
						prop.Transient = True
						splitStr = "SNAP_PROP_TRANSIENT("

					listed = line.split(splitStr)[1]
					listed = listed.split(");")[0].strip()
					listed = listed.split(',')
					
					# property is comma separated now

					prop.Type = listed[0].strip()
					prop.Name = listed[1].strip()

					# remove end paren if there are no configured details
					if (prop.Name.endswith(')')):
						prop.Name = prop.Name.rstrip(prop.Name[-1])

					# remove default value
					if '=' in prop.Name:
						s = prop.Name.split('=')
						prop.Name = s[0].strip()
						prop.DefaultValue = s[1].split(")")[0].split(",")[0].strip()					
					
					prop.Condition = ""
					if (len(listed) >= 3):
						# start at index 2, 0 and 1 are type and name
						for i in range(2, len(listed)):
							if listed[i].strip().startswith("SNAP_FIRST_ONLY"):
								prop.FirstOnly = True				
								continue	
							if listed[i].strip().startswith("SNAP_SEND_VIEWING"):
								prop.ViewOnly = True
								continue		
							if listed[i].strip().startswith("SNAP_DESTROY"):
								prop.Destroy = True
								continue			
							if listed[i].strip().startswith("SNAP_SKIP_REPLAY"):
								prop.SkipReplay = True
								continue
							if listed[i].strip().startswith("SNAP_REPLAY_ONLY"):
								prop.ReplayOnly = True
								continue
							if (listed[i].strip().startswith("SNAP_SEND_CONDITION")):
								prop.ConditionFunction = prop.Condition.split('(')[1].split(')')[0].strip()
								continue
							if listed[i].strip().startswith("SNAP_SEND_OWNER"):
								prop.OwnerOnly = True
								continue
							if listed[i].strip().startswith("SNAP_SEND_OTHERS"):
								prop.OthersOnly = True
								continue
							#prop.Condition = listed[i].strip()
						
						# special case for function condition
							#prop.Condition = "SNAP_SEND_CONDITION"
						
						# remove end paren for final condition
						#if (prop.Condition.endswith(')')):
						#	prop.Condition = prop.Condition.rstrip(prop.Condition[-1])

					classInfo.Properties.append(prop)

# add OnRep functions to properties
for info in classData:
	for prop in info.Properties:
		for func in info.OnRepFunctions:
			if func == prop.Name:
				prop.OnRepFunction = func
				break

# will be recursed
def findSnapshottableParent(info):

	for p in info.ParentNames:
		if p == "KSnapshottable":
			# directly inherits snapshottable
			info.IsSnapshottable = True
			return True
			
	for p in info.ParentNames:
		for i in classData:
			if i.ClassName == p:
				# this parent has a class info in this script
				if i.IsSnapshottable:
					# this parent was previously marked snapshottable
					info.IsSnapshottable = True
					info.SnapshotParent = i
					return True
				else:
					# see if this parent is snapshottable
					if findSnapshottableParent(i):
						info.SnapshotParent = i
						info.IsSnapshottable = True
						return True
	return False
						


for info in classData:
	if not info.IsSnapshottable:
		findSnapshottableParent(info)
	if info.IsSnapshottable and info.ClassName in instantiableClasses:
		info.HashName(0)


ownerProp = ClassInfo.Property()
ownerProp.Name = "OwningPlayerIndex"
ownerProp.Type = "u8"
ownerProp.DefaultValue = "NULL_PLAYER"
ownerProp.Destroy = True

def addParentProps(info: ClassInfo, parent: ClassInfo):
	for prop in parent.Properties:
		if prop not in info.Properties:
			info.Properties.insert(0, prop)
	for p in parent.ParentNames:
		for i in classData:
			if i.ClassName == parent:
				addParentProps(info, i)

# add parent properties
for info in classData:

	for parent in info.ParentNames:
		for i in classData:
			if i.ClassName == parent:
				addParentProps(info, i)
		

	#parent = info.SnapshotParent
	#while parent:
	#	for prop in parent.Properties:
	#		if prop not in info.Properties:
	#			info.Properties.insert(0, prop)
	#	parent = parent.SnapshotParent

	if ownerProp not in info.Properties:
		info.Properties.insert(0, ownerProp)
	else:
		# make sure its first
		ownerIndex = info.Properties.index(ownerProp)
		if ownerIndex != 0:
			info.Properties[ownerIndex], info.Properties[0] = info.Properties[0], info.Properties[ownerIndex]

# remove non-snappshottables
classData[:] = [info for info in classData if info.IsSnapshottable]

# sort by level of inheritance from KSnapshottable
sortedData = [None]
while len(sortedData) != len(classData) + 1:
	for info in classData:
		if info.SnapshotParent in sortedData:
			sortedData.append(info)

sortedData.remove(None)
classData = sortedData

################# write file #################

# parse states "enum"
FREE = 1
CLASS = 2
PROPS = 3
VARIABLE = 4

parseState = FREE

varNames = [
	"$SNAPPARENT",
	"$CLASSNAME",
	"$CLASSID",
	"$PROPCOUNT",
	"$DESTROY_REPCOUNT",
	"$ITERREP",
	"$HEADER",
	"$DESTROY_CHECKREPCOUNT",
	#"$CHECKREPCOUNT",
	"$DESTROY_DECLARECALLREP"
]

propVars = [
	"$PROPNAME",
	"$PROPTYPE",
	"$PROPDEFAULT",
	"$PROPINDEX",
	"$PROPCONDITION",
	"$PROPTRANS",
	"$COPYPROP",
	"$PROPDESTROY",
	"$PROPEQDEF",
	"$PACKPROP",
	"$APPLYPROP",
	"$ONREP",
	"$ADDONREP",
	"$DESTROY_ADDONREP",
	"$ADDREPCOUNT",
	"$LINEBREAK"
]

classChar = '@'
propsChar = '`'
varChar = '$'
symbols = [classChar, propsChar, varChar]

def TextFromVar(info: ClassInfo, varText: str):
	if varText == "$SNAPPARENT":
		if not info.SnapshotParent:
			return "KSnapshot"
		return info.SnapshotParent.GetStructName()
	if varText == "$CLASSNAME":
		return info.ClassName
	if varText == "$CLASSID":
		return info.Hash
	if varText == "$PROPCOUNT":
		return str(len(info.Properties))
	if varText == "$HEADER":
		return info.HeaderPath
	if varText == "$DESTROY_REPCOUNT":
		repCount = 0
		for prop in info.Properties:
			if prop.OnRepFunction:
				if prop.Destroy:
					repCount += 1
		if repCount > 0:
			return """u8 OnRepPending[""" + str(repCount) + """];
	u8 OnRepCount = 0;"""
		return ""
	if varText == "$REPCOUNT":
		repCount = 0
		for prop in info.Properties:
			if prop.OnRepFunction:			
				repCount += 1
		if repCount > 0:
			return """u8 OnRepPending[""" + str(repCount) + """];
	u8 OnRepCount = 0;"""
		return ""
	if varText == "$ITERREP":
		return """for (u8 i = 0; i < onRepCount; i++)
		CallOnRepForIndex(onRepPending[i], obj);"""
	if varText == "$DESTROY_DECLARECALLREP":
		for prop in info.Properties:
			if prop.Destroy and prop.OnRepFunction != "":
				return "void CallOnRepForIndex(u8 index, " + info.ClassName + "* obj);"
		return ""
	if varText == "$DESTROY_CHECKREPCOUNT":
		for prop in info.Properties:
			if prop.Destroy and prop.OnRepFunction != "":
				return "2"
		return "0"
	if varText == "$CHECKREPCOUNT":
		for prop in info.Properties:
			if prop.OnRepFunction != "":
				return "2"
		return "0"
	
def TextFromProp(prop: ClassInfo.Property, propText: str, info):
	if propText == "$ADDONREP":
		if not prop.OnRepFunction:
			return ""
		return """KPendingOnRep rep;
			rep.Object = object;
			rep.Index = """ + str(info.Properties.index(prop)) + """;
			PendingOnReps.push_back(rep);"""
	if propText == "$DESTROY_ADDONREP":
		if not prop.OnRepFunction:
			return ""
		return """KPendingOnRep rep;
			rep.Object = object;
			rep.Index = """ + str(info.Properties.index(prop)) + """;
			DestroyPendingOnReps.push_back(rep);"""			
	if propText == "$LINEBREAK":
		return "\n"
	if propText == "$PROPNAME":
		return prop.Name
	if propText == "$PROPTYPE":
		return prop.Type
	if propText == "$PROPDEFAULT":
		return prop.DefaultValue
	if propText == "$PROPINDEX":
		return str(info.Properties.index(prop))
	if propText == "$PROPTRANS":
		if prop.Transient:
			return " "
		else:
			return ""
	if propText == "$COPYPROP":
		if prop.Transient:
			return ""
		else:
			return "memcpy(&object->" + prop.Name + ", data, sizeof(" + prop.Type + "));"
	if propText == "$PROPDESTROY":
		if prop.Destroy:
			return " "
		return ""
	if propText == "$PROPEQDEF":
		if prop.DefaultValue:
			return " = " + prop.DefaultValue
		return ""
	if propText == "$ONREP":
		return prop.OnRepFunction
	if propText == "$ADDREPCOUNT":
		if prop.OnRepFunction:
			return """onRepPending[onRepCount] = """ + str(info.Properties.index(prop)) + """;
			onRepCount++;"""
		return ""
	if propText == "$PACKPROP":
		if prop.Transient:
			return  prop.Type + """  val;
		casted->GetTransient_""" + prop.Name + """(val);
		memcpy(snapAddr, &val, size);"""
		else:
			return"""u8* objAddr = (u8*)&casted->""" + prop.Name + """;
				memcpy(snapAddr, objAddr, size);"""
	if propText == "$APPLYPROP":
		if prop.Transient:
			return """obj->SetTransient_""" + prop.Name + """(""" + prop.Name + """);"""
		s = "memcpy(&obj->" + prop.Name + ", &" + prop.Name + ", sizeof(" + prop.Type + "));"
		if prop.OnRepFunction:
			s +=  """
		OnRepPending[OnRepCount] = """ + str(info.Properties.index(prop)) + """;
		OnRepCount++;"""
		return s
	if propText == "$PROPCONDITION":
		firstCheck = "(lastAcked < object->SnapshottableFrameCreated)"
		viewCheck = "VIEWCHECK"
		skipReplay = "!storingReplay"
		onlyReplay = "storingReplay"
		retText = ""
		
		if prop.FirstOnly:
			retText += firstCheck + " && "

		if prop.ViewOnly:
			retText += "" + " && "

		if prop.SkipReplay:
			retText += skipReplay + " && "

		if prop.ReplayOnly:
			retText += onlyReplay + " && "

		if prop.OwnerOnly:
			retText += "(playerIndex == object->OwningPlayerIndex || storingReplay) && "
		
		if prop.OthersOnly:
			retText += "(playerIndex != object->OwningPlayerIndex || storingReplay) && "

		if prop.ConditionFunction:
			retText += prop.ConditionFunction + "(playerIndex, object) && "

		if retText.endswith(" && "):
			retText = retText[:-4]

		if retText == "":
			retText = "true"

		return retText

def CheckPropIf(prop: ClassInfo.Property, propText: str):
	charBuff = ""
	ifBuff = ""
	ifScope = 0
	for c in propText:

		charBuff += c
			
		if charBuff[-3:] == "$IF":
			ifScope += 1
			charBuff = "$IF"
			continue
		if charBuff[-6:] == "$ENDIF":
			ifScope -= 1
			ifBuff = charBuff[:1] + charBuff[3:]
			# read until we get a var
			varBuff = ""
			found = False
			for char in ifBuff:
				if found:
					break
				varBuff += char
				for var in propVars:
					if var in varBuff:
						# found condition var
						if TextFromProp(prop, var, info) == "":
							return propText.replace(charBuff, "")
						else:
							return propText.replace("$IF" + varBuff[1:], "").replace("$ENDIF", "")
						found = True
						break
							
						
			charBuff = charBuff.replace("$ENDIF", "")
	return propText
			
def ParsePropsString(info: ClassInfo, text: str):
	global FREE
	global CLASS
	global PROPS
	global VARIABLE
	global classChar
	global propsChar
	global varChar
	global varNames
	global propVars
	global parseState

	finalText = ""
	for prop in info.Properties:
		propText = text
		for propVar in propVars:
			propText = CheckPropIf(prop, propText)
			propText = propText.replace(propVar, TextFromProp(prop, propVar, info))
		finalText += propText
	return finalText

def ParseClassString(text: str):
	global FREE
	global CLASS
	global PROPS
	global VARIABLE
	global classChar
	global propsChar
	global varChar
	global varNames
	global parseState

	finalText = ""
	for info in classData:
		if "$PLAYERCLASS" in text and info.ClassName == "KNetPlayer":
			continue
		if info.ClassName not in instantiableClasses:
			continue

		classText = text.replace("$PLAYERCLASS", "")
		for varName in varNames:
			classText = classText.replace(varName, TextFromVar(info, varName))
		
		charBuff = ""
		for c in classText:
			if c == propsChar:
				if parseState == CLASS:
					# begin parsing props
					parseState = PROPS
					charBuff = ""
					continue
				else:
					# finished parsing props
					assert parseState == PROPS, "parse state FREE while parsing class - line " + str(ln) + " col " + str(col)
					parseState = CLASS
					classText = classText.replace('`' + charBuff + '`', ParsePropsString(info, charBuff))
					charBuff = ""
					continue

			charBuff += c
				
		finalText += classText

	return finalText	

fileText = ""
with open ("net_format.txt", 'r') as file:
	charBuff = ""
	ln = 1
	col = 1
	while True:
		c = file.read(1)
		if not c:
			fileText += charBuff
			break
		
		# track position
		col += 1
		if c == '\n':
			ln += 1
			col = 1

		if parseState == FREE:
			if c not in symbols:
				charBuff += c
				continue
			
			assert c != varChar, "cannot parse a variable without a class - line " + str(ln) + " col " + str(col)
			assert c != propsChar, "cannot parse a props without a class - line " + str(ln) + " col " + str(col)
			
			# we know c == classChar
			fileText += charBuff
			charBuff = ""
			parseState = CLASS
			continue
		
		if parseState == CLASS:
			if c == classChar:
				fileText += ParseClassString(charBuff)
				assert parseState == CLASS, "parse state was incorrect after parsing class - line " + str(ln) + " col " + str(col)
				charBuff = ""
				parseState = FREE
				continue		
			charBuff += c
			continue


with open ("src/engine/net/snapshot.cpp", 'w') as file:
	for info in classData:
			if info.ClassName == "KNetPlayer":
				fileText = fileText.replace("%PLAYERID%", info.Hash)
				break
	file.write(fileText)

print("finished")