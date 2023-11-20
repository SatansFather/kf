#if _DEV
#if __linux__
#define RES_DIR std::string("../../../src/res/")
#else
#define RES_DIR std::string("../../../res/")
#endif
#else
#define RES_DIR std::string("res/")
#endif

#define SOUND_DIR std::string(RES_DIR + "sound/")
#define TEX_DIR std::string(RES_DIR + "textures/")
#define MODEL_DIR std::string(RES_DIR + "model/")

#if _DEV
#define CFG_DIR RES_DIR
#define MAP_DIR std::string(RES_DIR + "maps/")
#define SAVE_DIR std::string(RES_DIR + "saves/")
#define REPLAY_DIR std::string(RES_DIR + "replays/")
#define SCREENSHOT_DIR std::string(RES_DIR + "screenshots/")
#define FONT_DIR std::string(RES_DIR + "fonts/")
#define LUA_DIR std::string(RES_DIR + "lua/")
#define SRC_DIR std::string("../../../src/")
#define REPLAY_DIR std::string(RES_DIR + "replays/")

#define HLSL_DIR std::string("../../../src/engine/render/backend/d3d11/hlsl/")
#define GLSL_DIR std::string("../../../src/engine/render/backend/opengl/glsl/")

#else
#define MAP_DIR std::string("maps/")
#define SAVE_DIR std::string("saves/")
#define REPLAY_DIR std::string("replays/")
#define SCREENSHOT_DIR std::string("screenshots/")
#define CFG_DIR std::string("")
#define FONT_DIR std::string("fonts/")
#define LUA_DIR std::string("lua/")
#define REPLAY_DIR std::string("replays/")

#define HLSL_DIR std::string("shaders/hlsl/")
#define GLSL_DIR std::string("shaders/glsl/")

#endif



/*
#define RES_DIR std::string("res/")

#define SOUND_DIR std::string(RES_DIR + "sound/")
#define TEX_DIR std::string(RES_DIR + "textures/")
#define MODEL_DIR std::string(RES_DIR + "model/")

#if _DEV
#define CFG_DIR RES_DIR
#define MAP_DIR std::string(RES_DIR + "maps/")
#define SAVE_DIR std::string(RES_DIR + "saves/")
#define REPLAY_DIR std::string(RES_DIR + "replays/")
#define SCREENSHOT_DIR std::string(RES_DIR + "screenshots/")
#define FONT_DIR std::string(RES_DIR + "fonts/")

//#define HLSL_DIR std::string("../../../src/engine/render/backend/d3d11/hlsl/")
#define HLSL_DIR std::string(RES_DIR + "hlsl/")
#define GLSL_DIR std::string("../../../src/engine/render/backend/opengl/glsl/")

#else
#define MAP_DIR std::string("maps/")
#define SAVE_DIR std::string("saves/")
#define REPLAY_DIR std::string("replays/")
#define SCREENSHOT_DIR std::string("screenshots/")
#define CFG_DIR std::string("")
#define FONT_DIR std::string("fonts/")
#endif*/