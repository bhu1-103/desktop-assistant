local cjson = require("cjson")
local classifier = require("classifier")
local db = require("models/music-controls_embeddings")

local query = table.concat(arg, " ")

if query == "" then
	print('usage: luajit infer.lua "pause music"')
	os.exit(1)
end

-- Prepare the Ollama payload
local payload = cjson.encode({
	model = "snowflake-arctic-embed:22m",
	input = query,
})

-- Fetch embedding
local cmd = "curl -s http://localhost:11434/api/embed "
	.. '-H "Content-Type: application/json" '
	.. "-d "
	.. string.format("%q", payload)

local pipe = io.popen(cmd)
local response = pipe:read("*a")
pipe:close()

local embedding = cjson.decode(response).embeddings[1]

-- Run it through the classifier
local r = classifier.classify(embedding, db)

-- Output ONLY the final decision
if r.accepted and r.intent then
	print(r.intent)
	if r.intent == "PLAY_MUSIC" then
		os.execute("playerctl play")
		os.execute("pactl set-sink-mute @DEFAULT_SINK@ false")
		os.execute("pactl set-sink-volume @DEFAULT_SINK@ 30%")
	end
	if r.intent == "PAUSE_MUSIC" then
		os.execute("playerctl pause")
	end
	if r.intent == "NEXT_TRACK" then
		os.execute("playerctl next")
	end
	if r.intent == "PREVIOUS_TRACK" then
		os.execute("playerctl previous")
	end
	if r.intent == "VOLUME_UP" then
		os.execute("pactl set-sink-mute @DEFAULT_SINK@ false")
		os.execute("pactl set-sink-volume @DEFAULT_SINK@ +10%")
	end
	if r.intent == "VOLUME_DOWN" then
		os.execute("pactl set-sink-volume @DEFAULT_SINK@ -10%")
	end
else
	print("REJECT")
end
