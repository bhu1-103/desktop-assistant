local M = {}

local DIM = 384
local TOP_K = 5

-- Lowered margin to prevent vector collapse rejection, raised confidence slightly
local MIN_CONFIDENCE = 0.85
local MIN_MARGIN = 0.005

local function dot(a, b)
	local sum = 0

	for i = 1, DIM do
		sum = sum + a[i] * b[i]
	end

	return sum
end

local function insert_topk(top, item)
	top[#top + 1] = item

	table.sort(top, function(a, b)
		return a.score > b.score
	end)

	if #top > TOP_K then
		top[#top] = nil
	end
end

function M.classify(query, db)
	local neighbors = {}

	-- Find top 5 neighbors.
	for _, item in ipairs(db) do
		insert_topk(neighbors, {
			score = dot(query, item.embedding),
			label = item.label,
			text = item.text,
		})
	end

	if #neighbors == 0 then
		return {
			intent = nil,
			confidence = 0,
			margin = 0,
			accepted = false,
			neighbors = {},
			best = nil,
			second = nil,
		}
	end

	-- Count labels, sum scores, and remember best similarity per label.
	local labels = {}

	for _, n in ipairs(neighbors) do
		local x = labels[n.label]

		if not x then
			x = {
				count = 0,
				score_sum = 0,
				best = n.score,
				best_text = n.text,
			}

			labels[n.label] = x
		end

		x.count = x.count + 1
		x.score_sum = x.score_sum + n.score

		if n.score > x.best then
			x.best = n.score
			x.best_text = n.text
		end
	end

	-- Rank labels:
	-- 1. Highest individual similarity (best) wins.
	-- 2. If tied, highest combined similarity (score_sum) wins.
	local ranked = {}

	for label, x in pairs(labels) do
		ranked[#ranked + 1] = {
			label = label,
			count = x.count,
			score_sum = x.score_sum,
			best = x.best,
			best_text = x.best_text,
		}
	end

	table.sort(ranked, function(a, b)
		if a.best ~= b.best then
			return a.best > b.best
		end

		return a.score_sum > b.score_sum
	end)

	local best = ranked[1]
	local second = ranked[2]

	local confidence = best.best
	local second_best = second and second.best or 0
	local margin = confidence - second_best

	local accepted = confidence >= MIN_CONFIDENCE and margin >= MIN_MARGIN

	return {
		intent = accepted and best.label or nil,
		confidence = confidence,
		margin = margin,
		accepted = accepted,

		best = best,
		second = second,

		ranked = ranked,
		neighbors = neighbors,
	}
end

return M
