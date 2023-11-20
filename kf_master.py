from flask import Flask, request
import json
import time

app = Flask(__name__)
serverTable = {}

@app.route('/info', methods=["GET", "POST"])
def info():
	global serverTable
	if request.method == "GET":

		remove = [s for s in serverTable if time.time() - serverTable[s]['time'] > 30]
		for s in remove: del serverTable[s]

		return json.dumps(serverTable)
	
	if request.method == "POST":
		ip = request.environ['REMOTE_ADDR']
		j = request.get_json(force=True)
		key = ip + ":" + j['port']
		j['time'] = time.time()
		serverTable[key] = j
		return ""

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0', port=6667)