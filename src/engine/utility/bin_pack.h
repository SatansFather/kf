#pragma once

#include "engine/global/types_numeric.h"

class KBinPacker
{
public:

	struct Vec2u
	{
		u32 x = 0, y = 0;
	};

	struct Rect
	{
		Vec2u pos;
		Vec2u size;
		void* Data = nullptr;
	};

private:

	struct Node
	{
		Node* children[2] = { nullptr, nullptr };
		Rect rect;
		bool filled = false;

		Node() = default;
		Node(const Node& other) = delete;
		Node& operator=(const Node& other) = delete;
		~Node() {
			if (children[0]) delete children[0];
			if (children[1]) delete children[1];
		}
		bool insert(Rect& dst_rect, const Vec2u& image_size)
		{
			// never one and not the other
			assert((children[0] != nullptr &&
				children[1] != nullptr) || (
					children[0] == nullptr &&
					children[1] == nullptr));

			if (children[0])
			{
				// try to add it to both children
				bool status = children[0]->insert(dst_rect, image_size);

				if (status) { return true; }

				return children[1]->insert(dst_rect, image_size);
			}
			else
			{
				// we're a leaf, but already full :(
				if (filled) { return false; }

				// the new image doesn't fit
				if (image_size.x > rect.size.x || image_size.y > rect.size.y)
				{
					return false;
				}

				// the image fits perfectly. easy.
				// in fact, this is the only route to successful insertion
				if (image_size.x == rect.size.x && image_size.y == rect.size.y)
				{
					//dst_rect = rect;
					dst_rect.pos.x = rect.pos.x;
					dst_rect.pos.y = rect.pos.y;
					filled = true;
					return true;
				}

				children[0] = new Node;
				children[1] = new Node;

				// compare difference in dimensions and decide which way to split
				int dw = rect.size.x - image_size.x;
				int dh = rect.size.y - image_size.y;

				if (dw > dh)
				{
					// vertical split with left node width equal to incoming image's
					children[0]->rect.pos.x = this->rect.pos.x;
					children[0]->rect.pos.y = this->rect.pos.y;
					children[0]->rect.size.x = image_size.x;
					children[0]->rect.size.y = this->rect.size.y;

					children[1]->rect.pos.x = this->rect.pos.x + image_size.x;
					children[1]->rect.pos.y = this->rect.pos.y;
					children[1]->rect.size.x = this->rect.size.x - image_size.x;
					children[1]->rect.size.y = this->rect.size.y;
				}
				else
				{
					// horizontal split with top node height equal to incoming image's
					children[0]->rect.pos.x = this->rect.pos.x;
					children[0]->rect.pos.y = this->rect.pos.y;
					children[0]->rect.size.x = this->rect.size.x;
					children[0]->rect.size.y = image_size.y;

					children[1]->rect.pos.x = this->rect.pos.x;
					children[1]->rect.pos.y = this->rect.pos.y + image_size.y;
					children[1]->rect.size.x = this->rect.size.x;
					children[1]->rect.size.y = this->rect.size.y - image_size.y;
				}

				// this absolutely should work
				children[0]->insert(dst_rect, image_size);
				return true;
			}
		}
	};

	Node root;

public:

	void set_size(const Vec2u& size)
	{
		this->root.rect.size = size;
	}

	// Returns the resulting rect in dst_rect
	// Returns true if the operation was successful, false otherwise
	bool insert(Rect& dst_rect, const Vec2u& image_size)
	{
		return root.insert(dst_rect, image_size);
	}
};