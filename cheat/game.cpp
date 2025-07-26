#include "game.h"
#include "../menu/menu.h"
#include "../gui/Color_t.hpp"
#include "../spoofer/callstack.h"
#include "../protect/vmp.h"

#define M_PI   3.141592654f

unsigned char tempRainbow[4] = { 0, 15, 244, 0 };

void updateRainbowColor() {
	int stage = tempRainbow[3];
	switch (stage) {
	case 0:        tempRainbow[2] += 5;        if (tempRainbow[2] == 255)            tempRainbow[3] = 1;        break;
	case 1:        tempRainbow[0] -= 5;        if (tempRainbow[0] == 0)            tempRainbow[3] = 2;        break;
	case 2:        tempRainbow[1] += 5;        if (tempRainbow[1] == 255)            tempRainbow[3] = 3;        break;
	case 3:        tempRainbow[2] -= 5;        if (tempRainbow[2] == 0)            tempRainbow[3] = 4;        break;
	case 4:        tempRainbow[0] += 5;        if (tempRainbow[0] == 255)            tempRainbow[3] = 5;        break;
	case 5:        tempRainbow[1] -= 5;        if (tempRainbow[1] == 0)            tempRainbow[3] = 0;        break;
	}
}

bool in_game()
{
	auto gameMode = *(int*)(game_data->game_offsets.gamemode);
	return  gameMode != 0;
}

void draw_line(const ImVec2& from, const ImVec2& to, ImU32 color, float thickness)
{
	ImGui::GetWindowDrawList()->AddLine(from, to, color, thickness);
}

void rect_filled(const Vector2& pos, const Vector2& size, const col_t& col)
{
	ImGui::GetWindowDrawList()->AddRectFilled({ pos.x, pos.y }, { pos.x + size.x, pos.y + size.y }, ImColor(col.r(), col.g(), col.b(), col.a()));
}

void player_rect_filled(const Vector2& pos, const Vector2& size, ImU32 col)
{
	ImGui::GetWindowDrawList()->AddRectFilled({ pos.x, pos.y }, { pos.x + size.x, pos.y + size.y }, col);
}

Vector2 get_screen_size()
{
	return Vector2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);
}

void draw_crosshair_cb()
{
	constexpr auto width = 1, height = 8;

	if (game_data->settings.rgbcrosshair)
	{
		game_data->settings.crosshaircolorint = ImGui::GetColorU32(ImVec4((float)tempRainbow[0] / 255, (float)tempRainbow[1] / 255, (float)tempRainbow[2] / 255, 255 / 255));
	}
	else
	{
		game_data->settings.crosshaircolorint = ImGui::ColorConvertFloat4ToU32(game_data->settings.crosshaircolor);
	}

	player_rect_filled(get_screen_size() / 2 - Vector2{ width + 1, height + 1 }, { (width + 1) * 2, (height + 1) * 2 }, IM_COL32(0, 0, 0, 255));
	player_rect_filled(get_screen_size() / 2 - Vector2{ height + 1, width + 1 }, { (height + 1) * 2, (width + 1) * 2 }, IM_COL32(0, 0, 0, 255));

	player_rect_filled(get_screen_size() / 2 - Vector2{ width, height }, { width * 2, height * 2 }, game_data->settings.crosshaircolorint);
	player_rect_filled(get_screen_size() / 2 - Vector2{ height, width }, { height * 2, width * 2 }, game_data->settings.crosshaircolorint);
}

void RotatePoint(ImVec2& point, const ImVec2& center, float angle)
{
	float s = sin(angle);
	float c = cos(angle);

	// Translate point back to the origin
	point.x -= center.x;
	point.y -= center.y;

	// Rotate point
	float xnew = point.x * c - point.y * s;
	float ynew = point.x * s + point.y * c;

	// Translate point back
	point.x = xnew + center.x;
	point.y = ynew + center.y;
}

void draw_spinny_crosshair()
{
	constexpr float crosshair_size = 8;
	ImVec2 center = ImVec2(ImGui::GetIO().DisplaySize.x / 2.f, ImGui::GetIO().DisplaySize.y / 2.f);
	ImU32 outline_color = ImGui::GetColorU32(ImVec4(0, 0, 0, 1)); // Black outline color

	if (game_data->settings.rgbcrosshair)
	{
		game_data->settings.crosshaircolorint = ImGui::GetColorU32(ImVec4((float)tempRainbow[0] / 255, (float)tempRainbow[1] / 255, (float)tempRainbow[2] / 255, 255 / 255));
	}
	else
	{
		game_data->settings.crosshaircolorint = ImGui::ColorConvertFloat4ToU32(game_data->settings.crosshaircolor);
	}

	// Update the rotation angle (you can adjust the rotation speed as needed)
	float rotationAngle = fmod(ImGui::GetTime(), 360.0f);  // Example: Rotate every second

	// Draw the rotated horizontal line of the cross
	ImVec2 startPointH = ImVec2(center.x - 9, center.y);
	ImVec2 endPointH = ImVec2(center.x + 9, center.y);
	RotatePoint(startPointH, center, rotationAngle);
	RotatePoint(endPointH, center, rotationAngle);
	draw_line(startPointH, endPointH, outline_color, 4);

	// Draw the rotated vertical line of the cross
	ImVec2 startPointV = ImVec2(center.x, center.y - 9);
	ImVec2 endPointV = ImVec2(center.x, center.y + 9);
	RotatePoint(startPointV, center, rotationAngle);
	RotatePoint(endPointV, center, rotationAngle);
	draw_line(startPointV, endPointV, outline_color, 4);

	// Draw the rotated horizontal line of the cross
	ImVec2 startPointHRotated = ImVec2(center.x - crosshair_size, center.y);
	ImVec2 endPointHRotated = ImVec2(center.x + crosshair_size, center.y);
	RotatePoint(startPointHRotated, center, rotationAngle);
	RotatePoint(endPointHRotated, center, rotationAngle);
	draw_line(startPointHRotated, endPointHRotated, game_data->settings.crosshaircolorint, 2);

	// Draw the rotated vertical line of the cross
	ImVec2 startPointVRotated = ImVec2(center.x, center.y - crosshair_size);
	ImVec2 endPointVRotated = ImVec2(center.x, center.y + crosshair_size);
	RotatePoint(startPointVRotated, center, rotationAngle);
	RotatePoint(endPointVRotated, center, rotationAngle);
	draw_line(startPointVRotated, endPointVRotated, game_data->settings.crosshaircolorint, 2);
}

//void rect(const Vector2& pos, const Vector2& size, const C_Color& col)
//{
//	ImGui::GetWindowDrawList()->AddRect({ pos.x, pos.y }, { pos.x + size.x, pos.y + size.y }, ImColor(col.r(), col.g(), col.b(), col.a()));
//}
void rect(const Vector2& pos, const Vector2& size, ImU32 col)
{
	ImGui::GetWindowDrawList()->AddRect({ pos.x, pos.y }, { pos.x + size.x, pos.y + size.y }, col);
}

void draw_corner_box(const ImVec2& top_left, const ImVec2& bottom_right, ImU32 color, float thickness = 1.0f)
{
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	float line_length = (bottom_right.x - top_left.x) / 4.0f; // Adjust the line length for the corners

	// Top left corner
	drawList->AddLine(top_left, ImVec2(top_left.x + line_length, top_left.y), color, thickness);
	drawList->AddLine(top_left, ImVec2(top_left.x, top_left.y + line_length), color, thickness);

	// Top right corner
	drawList->AddLine(ImVec2(bottom_right.x, top_left.y), ImVec2(bottom_right.x - line_length, top_left.y), color, thickness);
	drawList->AddLine(ImVec2(bottom_right.x, top_left.y), ImVec2(bottom_right.x, top_left.y + line_length), color, thickness);

	// Bottom left corner
	drawList->AddLine(ImVec2(top_left.x, bottom_right.y), ImVec2(top_left.x + line_length, bottom_right.y), color, thickness);
	drawList->AddLine(ImVec2(top_left.x, bottom_right.y), ImVec2(top_left.x, bottom_right.y - line_length), color, thickness);

	// Bottom right corner
	drawList->AddLine(bottom_right, ImVec2(bottom_right.x - line_length, bottom_right.y), color, thickness);
	drawList->AddLine(bottom_right, ImVec2(bottom_right.x, bottom_right.y - line_length), color, thickness);
}

std::string ConvertDistanceToString(float dist)
{
	return std::to_string(static_cast<int>(dist)) + XOR("M");
}

ImVec2 AddVecs(Vector2 vec2, ImVec2 imVec2)
{
	ImVec2 returnValue{};
	returnValue.x = vec2.x + imVec2.x;
	returnValue.y = vec2.y + imVec2.y;
	return returnValue;
}

void drawString(Vector2 position, const char* string, ImColor colorText, ImColor colorBG, ImColor colorUnderLine)
{
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	drawList->AddText(ImVec2(position.x + 2.5f, position.y), colorText, string);
}

//void LootThread()
//{
//	if (game_data->settings.checkPassed && game_data->settings.b_ingame && game_data->settings.loot_test)
//	{
//		for (int i = 0; i < itemCount; i++)
//		{
//			if (!loot_t[i].canDrawLoot)
//				continue;
//
//			if (game_data->settings.loot_test)
//			{
//				if ((game_data->settings.lootammo && loot_t[i].itemType == ItemType::TYPE_AMMO) ||
//					(game_data->settings.lootweapons && loot_t[i].itemType == ItemType::TYPE_WEAPONS) ||
//					(game_data->settings.lootmoney && loot_t[i].itemType == ItemType::TYPE_CASH) ||
//					(game_data->settings.lootarmor && loot_t[i].itemType == ItemType::TYPE_ARMOR) ||
//					(game_data->settings.lootmisc && loot_t[i].itemType == ItemType::TYPE_MISC) ||
//					(game_data->settings.lootsupply && loot_t[i].itemType == ItemType::TYPE_SUPPLY))
//				{
//					drawString(loot_t[i].screenPos, loot_t[i].name.c_str(), loot_t[i].color, ImColor(0, 0, 0, 255), ImColor(0, 0, 0, 255));
//
//					if (game_data->settings.loot_bounds_3d)
//					{
//						if ((game_data->settings.lootammo && loot_t[i].itemType == ItemType::TYPE_AMMO) ||
//							(game_data->settings.lootweapons && loot_t[i].itemType == ItemType::TYPE_WEAPONS) ||
//							(game_data->settings.lootmoney && loot_t[i].itemType == ItemType::TYPE_CASH) ||
//							(game_data->settings.lootarmor && loot_t[i].itemType == ItemType::TYPE_ARMOR) ||
//							(game_data->settings.lootmisc && loot_t[i].itemType == ItemType::TYPE_MISC) ||
//							(game_data->settings.lootsupply && loot_t[i].itemType == ItemType::TYPE_SUPPLY))
//						{
//							if (game_data->settings.loot_box_type == 0)
//							{
//								draw_3d_box(loot_t[i].bounds, loot_t[i].origin, loot_t[i].angle, loot_t[i].color);
//							}
//							else if (game_data->settings.loot_box_type == 1)
//							{
//								draw_3d_box_filled(loot_t[i].bounds, loot_t[i].origin, loot_t[i].angle, loot_t[i].color);
//							}
//						}
//					}
//				}
//			}
//
//			if (game_data->settings.loot_cache)
//			{
//				for (int i = 0; i < cacheCount; i++)
//				{
//					if (!cache_t[i].canDrawLoot)
//						continue;
//
//					Vector2 basePos = cache_t[i].screenPos;
//
//					for (int j = 0; j < cache_t[i].count; j++)
//					{
//						Vector2 itemPos = basePos;
//						if (game_data->settings.lootsupply)
//						{
//							if (j == 0)
//							{
//								itemPos.y += 15;
//							}
//							else
//								itemPos.y += j * 15 + 15;
//						}
//						else
//						{
//							itemPos.y += j * 15;
//						}
//
//						drawString(itemPos, cache_t[i].name[j].c_str(), cache_t[i].color[j], ImColor(0, 0, 0, 255), ImColor(0, 0, 0, 255));
//					}
//				}
//			}
//		}
//	}
//}

 //Inside your ImGui rendering function (e.g., in the frame update loop)
//void RenderPlayerStateWindow()
//{
//	// Set window background to grey before starting the window
//	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));  // grey color
//
//	// Create a new window with a grey background
//	ImGui::Begin("Debug", NULL, ImGuiWindowFlags_AlwaysAutoResize);
//
//	// Display player state information
//	ImGui::Text("Command Time: %d", client::localPlayer->commandTime);
//	ImGui::Text("Server Time: %d", client::localPlayer->serverTime);
//	ImGui::Text("Local Position (Origin): (%.2f, %.2f, %.2f)", client::localPlayer->origin.x, client::localPlayer->origin.y, client::localPlayer->origin.z);
//	ImGui::Text("Local Velocity: (%.2f, %.2f, %.2f)", client::localPlayer->velocity.x, client::localPlayer->velocity.y, client::localPlayer->velocity.z);
//	ImGui::Text("Delta Angles: (%.2f, %.2f, %.2f)", client::localPlayer->deltaAngles.x, client::localPlayer->deltaAngles.y, client::localPlayer->deltaAngles.z);
//	ImGui::Text("Local Client Number: %u", client::localPlayer->clientNum);
//	ImGui::Text("View Angles: (%.2f, %.2f, %.2f)", client::localPlayer->viewAngles.x, client::localPlayer->viewAngles.y, client::localPlayer->viewAngles.z);
//	ImGui::Text("Damage Event: %u", client::localPlayer->damageEvent);
//	ImGui::Text("Damage Yaw: %u", client::localPlayer->damageYaw);
//	ImGui::Text("Damage Pitch: %u", client::localPlayer->damagePitch);
//	ImGui::Text("Damage Count: %u", client::localPlayer->damageCount);
//
//	// Displaying weapon state
//	for (int i = 0; i < 2; ++i)
//	{
//		ImGui::Text("Weapon State %d: %d", i, client::localPlayer->weapState[i].weaponState);
//	}
//
//	// End the window and pop style color
//	ImGui::End();
//	ImGui::PopStyleColor();
//}

namespace render {
	constexpr int FONT_CENTER_X = 1 << 0;
	constexpr int FONT_CENTER_Y = 1 << 1;
	constexpr int FONT_RIGHT = 1 << 2; // New constant for right alignment

	void text(ImVec2 pos, ImColor color, int flags, const char* text) {
		if (flags & FONT_CENTER_X) {
			pos.x -= ImGui::CalcTextSize(text).x / 2.0f;
		}
		if (flags & FONT_CENTER_Y) {
			pos.y -= ImGui::CalcTextSize(text).y / 2.0f;
		}
		if (flags & FONT_RIGHT) {
			pos.x -= ImGui::CalcTextSize(text).x; // Align text to the right
		}

		ImGui::GetWindowDrawList()->AddText(pos, color, text);
	}
}

// Shadow color
ImColor shadowColor(10, 10, 10, 255);

void RenderSnapline(const Vector2& screen_pos, int snapline_pos, ImU32 col)
{
	// Get screen dimensions
	ImVec2 screen_size = ImGui::GetIO().DisplaySize;

	// Determine the start position of the snapline
	ImVec2 start_pos;
	switch (snapline_pos)
	{
	case 0: // Top
		start_pos = ImVec2(screen_size.x / 2, 0);
		break;
	case 1: // Center
		start_pos = ImVec2(screen_size.x / 2, screen_size.y / 2);
		break;
	case 2: // Bottom
		start_pos = ImVec2(screen_size.x / 2, screen_size.y);
		break;
	default:
		return;
	}

	// Draw the snapline using the pre-calculated 2D position
	ImGui::GetForegroundDrawList()->AddLine(start_pos, ImVec2(screen_pos.x, screen_pos.y), col, 1.0f);
}

void LootThread()
{
	if (game_data->settings.checkPassed && game_data->settings.b_ingame && game_data->settings.loot_test)
	{
		for (const auto& loot : lootData)
		{
			if (!loot.canDrawLoot || loot.name.empty())
				continue;

			std::string displayText = loot.name;
			ImVec2 textSize = ImGui::CalcTextSize(displayText.c_str());

			// Center loot name
			Vector2 centeredPos = loot.screenPos;
			centeredPos.x -= textSize.x / 2.0f;

			Vector2 shadowPos = centeredPos;
			shadowPos.x += 1.0f;
			shadowPos.y += 1.0f;
			drawString(shadowPos, displayText.c_str(), ImColor(10, 10, 10, 255), ImColor(0, 0, 0, 255), ImColor(0, 0, 0, 255));

			drawString(centeredPos, displayText.c_str(), loot.color, ImColor(0, 0, 0, 255), ImColor(0, 0, 0, 255));

			// Center distance text
			displayText = std::to_string(int(loot.distance)) + XOR("M");
			ImVec2 distanceTextSize = ImGui::CalcTextSize(displayText.c_str());

			Vector2 distancePos = loot.screenPos;
			distancePos.y += 15.0f;
			distancePos.x -= distanceTextSize.x / 2.0f; // Center horizontally

			Vector2 shadowDistancePos = distancePos;
			shadowDistancePos.x += 1.0f;
			shadowDistancePos.y += 1.0f;
			drawString(shadowDistancePos, displayText.c_str(), ImColor(10, 10, 10, 255), ImColor(0, 0, 0, 255), ImColor(0, 0, 0, 255));

			drawString(distancePos, displayText.c_str(), loot.color, ImColor(0, 0, 0, 255), ImColor(0, 0, 0, 255));

			if (game_data->settings.loot_bounds_3d)
			{
				for (size_t i = 0; i + 1 < loot.boxEdges.size(); i += 2)
				{
					draw_line(ImVec2(loot.boxEdges[i].x, loot.boxEdges[i].y),
						ImVec2(loot.boxEdges[i + 1].x, loot.boxEdges[i + 1].y),
						loot.color, 1.f);
				}
			}
		}
	}
}

void ESPThread()
{
	if (game_data->settings.checkPassed && game_data->settings.b_ingame)
	{
		if (game_data->settings.AIMBOTFOVCIRCLE)
		{
			if (game_data->settings.FOV_rgb)
			{
				game_data->settings.FOVCOLOR = game_data->settings.chamColorRGB;
			}
			else
			{
				game_data->settings.FOVCOLOR = ImGui::ColorConvertFloat4ToU32(game_data->settings.FOVCOLOR_colorVec4);
			}
			ImVec2 center = ImVec2(ImGui::GetIO().DisplaySize.x / 2.f, ImGui::GetIO().DisplaySize.y / 2.f);
			ImGui::GetWindowDrawList()->AddCircle(center, client::radius, game_data->settings.FOVCOLOR, 100, 1.f);
		}

		if (game_data->settings.b_crosshair)
		{
			if (game_data->settings.b_spinny_crosshair)
			{
				draw_spinny_crosshair();
			}
			else
			{
				draw_crosshair_cb();
			}
		}

		if (game_data->settings.uavtest)
			radar::draw();

		if (game_data->settings.ShowSpectator)
		{
			char distance_buf[255];
			sprintf_s(distance_buf, XOR("Spectators : %d"), game_data->settings.activespec);

			// Get the screen width dynamically (assuming a function or variable provides this)
			ImVec2 display_size = ImGui::GetIO().DisplaySize;
			float x_pos = display_size.x - 10.0f; // 10 pixels from the right edge
			float y_pos = 10.0f; // 10 pixels from the top edge

			// Calculate text size for centering
			ImVec2 text_size = ImGui::CalcTextSize(distance_buf);

			// Render distance shadow text for better visibility
			render::text(
				{ x_pos + 1 - text_size.x, y_pos + 1 },
				ImColor(10, 10, 10, 255), // Shadow color
				render::FONT_CENTER_X | render::FONT_CENTER_Y,
				distance_buf
			);

			// Render actual distance text
			render::text(
				{ x_pos - text_size.x, y_pos },
				ImColor(255, 255, 255, 255), // Text color
				render::FONT_CENTER_X | render::FONT_CENTER_Y,
				distance_buf
			);
		}

		for (size_t i = 0; i <= 2046; i++)
		{
			player* p = &players[i];

			if (!p->is_valid)
				continue;

			if (p->distance >= game_data->settings.distanceFloat)
				continue;

			if (game_data->settings.enemies_only && p->team == players[client::localPlayer->clientNum].team)
				continue;

			if (p->is_AI && !(game_data->settings.showaibool))
				continue;

			// Check if this player is the best target
			bool isBestTarget = (game_data->settings.aim_target_esp && p == bestTarget);
			bool isVisible = (game_data->settings.b_visible && p->visible && !isBestTarget);

			// Assign colors normally
			ImU32 distanceColor = isVisible
				? ImGui::ColorConvertFloat4ToU32(game_data->settings.V_distance_colorVec4)
				: ImGui::ColorConvertFloat4ToU32(game_data->settings.distance_colorVec4);

			ImU32 boxColor = isVisible
				? ImGui::ColorConvertFloat4ToU32(game_data->settings.vis_boxnotFilled_colorVec4)
				: ImGui::ColorConvertFloat4ToU32(game_data->settings.boxnotFilled_colorVec4);

			ImU32 boneColor = isVisible
				? ImGui::ColorConvertFloat4ToU32(game_data->settings.V_skeleton_colorVec4)
				: ImGui::ColorConvertFloat4ToU32(game_data->settings.skeleton_colorVec4);

			ImU32 SnaplineColor = isVisible ? ImGui::ColorConvertFloat4ToU32(game_data->settings.V_Snaplines_colorVec4) : ImGui::ColorConvertFloat4ToU32(game_data->settings.Snaplines_colorVec4);

			// If this player is the best target, override with target color
			if (isBestTarget && bestTarget != nullptr && players[client::localPlayer->clientNum].player_name.health != 0)
			{
				ImU32 targetColor = ImGui::ColorConvertFloat4ToU32(game_data->settings.Target_color);
				distanceColor = targetColor;
				boxColor = targetColor;
				boneColor = targetColor;
				SnaplineColor = targetColor;
			}

			bool showDistanceESP = game_data->settings.distance_esp;
			bool showWeaponESP = game_data->settings.weapon_esp;
			bool showKillsESP = game_data->settings.kills_esp;
			bool showDeathsESP = game_data->settings.deaths_esp;
			bool showUnfilledBoxes = game_data->settings.unfilled_boxes;
			bool showFilledBoxes = game_data->settings.filled_boxes;
			bool showPlayer3DBoxes = game_data->settings.b_player_3dboxes || game_data->settings.b_player_filled_3dboxes;
			bool showCornerBox = game_data->settings.corner_box;
			bool showNameESP = game_data->settings.name_esp;
			bool showTeamIDESP = game_data->settings.teamid_esp;
			bool showHealthESP = game_data->settings.health_esp;
			//bool showSnaplines = game_data->settings.snaplines;
			bool showBoneESP = game_data->settings.bone_esp;
			bool showHeadESP = game_data->settings.head_esp;
			//bool showCompassESP = game_data->settings.compass_esp;
			bool showRealCompass = game_data->settings.realCompass;
			bool bubble_esp = game_data->settings.bubble_esp;

			if (p->bbox_is_valid)
			{
				if (showDistanceESP)
				{
					char distance_buf[255];
					sprintf_s(distance_buf, XOR("%s"), ConvertDistanceToString((int)p->distance).c_str());

					// Render distance shadow text for better visibility
					render::text(
						{ p->bbox.x + p->bbox.w * 0.5f + 1, p->bbox.y + p->bbox.z + 21 + 1 },
						ImColor(10, 10, 10, 255), // Shadow color
						render::FONT_CENTER_X | render::FONT_CENTER_Y,
						distance_buf
					);

					// Render actual distance text
					render::text(
						{ p->bbox.x + p->bbox.w * 0.5f, p->bbox.y + p->bbox.z + 21 },
						distanceColor, // Main text color
						render::FONT_CENTER_X | render::FONT_CENTER_Y,
						distance_buf
					);
				}

				if (showWeaponESP)
				{
					char weapon_buf[255];
					sprintf_s(weapon_buf, XOR("%s"), p->weaponDisplayName);

					// Render distance shadow text for better visibility
					render::text(
						{ p->bbox.x + p->bbox.w * 0.5f + 1, p->bbox.y + p->bbox.z + 7 + 1 },
						ImColor(10, 10, 10, 255), // Shadow color
						render::FONT_CENTER_X | render::FONT_CENTER_Y,
						weapon_buf
					);

					// Render actual distance text
					render::text(
						{ p->bbox.x + p->bbox.w * 0.5f, p->bbox.y + p->bbox.z + 7 },
						distanceColor, // Main text color
						render::FONT_CENTER_X | render::FONT_CENTER_Y,
						weapon_buf
					);
				}

				if (game_data->settings.Snapline_ESP)
				{
					RenderSnapline(p->bone_array.origin, game_data->settings.Snapline_type, SnaplineColor);
				}

				if (showKillsESP)
				{
					char buf[255];
					sprintf_s(buf, XOR("Kills: %d"), p->kills);

					render::text(
						{ p->bbox.x + p->bbox.w * 0.5f + 1, p->bbox.y + p->bbox.z + 35 + 1 },
						ImColor(10, 10, 10, 255),
						render::FONT_CENTER_X | render::FONT_CENTER_Y,
						buf
					);

					// Render actual distance text
					render::text(
						{ p->bbox.x + p->bbox.w * 0.5f, p->bbox.y + p->bbox.z + 35 },
						distanceColor, // Main text color
						render::FONT_CENTER_X | render::FONT_CENTER_Y,
						buf
					);
				}

				if (showDeathsESP)
				{
					char buf[255];
					sprintf_s(buf, XOR("Deaths: %d"), p->deaths);

					// Render distance shadow text for better visibility
					render::text(
						{ p->bbox.x + p->bbox.w * 0.5f + 1, p->bbox.y + p->bbox.z + 49 + 1 },
						ImColor(10, 10, 10, 255), // Shadow color
						render::FONT_CENTER_X | render::FONT_CENTER_Y,
						buf
					);

					// Render actual distance text
					render::text(
						{ p->bbox.x + p->bbox.w * 0.5f, p->bbox.y + p->bbox.z + 49 },
						distanceColor, // Main text color
						render::FONT_CENTER_X | render::FONT_CENTER_Y,
						buf
					);
				}

				if (showUnfilledBoxes)
				{
					rect({ p->bbox.x - 1, p->bbox.y - 1 }, { p->bbox.w + 2, p->bbox.z + 2 }, IM_COL32(10, 10, 10, 255));
					rect({ p->bbox.x + 1, p->bbox.y + 1 }, { p->bbox.w - 2, p->bbox.z - 2 }, IM_COL32(10, 10, 10, 255));
					rect({ p->bbox.x, p->bbox.y }, { p->bbox.w, p->bbox.z }, boxColor);
				}

				if (showFilledBoxes)
				{
					player_rect_filled({ p->bbox.x, p->bbox.y }, { p->bbox.w, p->bbox.z }, game_data->settings.filledboxColor);
				}

				if (showPlayer3DBoxes)
				{
					ImVec3 angles = getAngles(i);
					Vector3 anglesVec(angles.x, angles.y, angles.z);

					if (game_data->settings.b_player_3dboxes)
					{
						draw_3d_box(p->Player_Bounds, p->playerPos, anglesVec, boxColor);
					}

					if (game_data->settings.b_player_filled_3dboxes)
					{
						draw_3d_box_filled(p->Player_Bounds, p->playerPos, anglesVec, boxColor);
					}
				}

				if (showCornerBox)
				{
					// Extract coordinates and size from p->bbox
					ImVec2 top_left = { p->bbox.x, p->bbox.y };
					ImVec2 bottom_right = { p->bbox.x + p->bbox.w, p->bbox.y + p->bbox.z };

					// Draw the corner box
					draw_corner_box(top_left, bottom_right, boxColor);
				}

				if (showNameESP)
				{
					char buf[255];

					if (!p->is_AI)
					{
						sprintf_s(buf, XOR("%s"), p->player_name.name);
					}
					else
					{
						sprintf_s(buf, XOR("%s"), XOR("Zombie"));
					}


					// Render player name shadow text for better visibility
					render::text(
						{ p->bbox.x + p->bbox.w * 0.5f + 1, p->bbox.y - 7 + 1 },
						ImColor(10, 10, 10, 255),
						render::FONT_CENTER_X | render::FONT_CENTER_Y,
						buf
					);

					// Render actual player name text
					render::text(
						{ p->bbox.x + p->bbox.w * 0.5f, p->bbox.y - 7 },
						distanceColor,
						render::FONT_CENTER_X | render::FONT_CENTER_Y,
						buf
					);
				}

				if (bubble_esp)
				{
					ImGui::GetWindowDrawList()->AddCircleFilled({ p->bone_array.j_helmetpos_screen.x, p->bone_array.j_helmetpos_screen.y + -20 }, 5, distanceColor, 100);
				}

				if (showTeamIDESP)
				{
					char buf[255];
					sprintf_s(buf, XOR("%d"), p->team);

					// Shadow text for better visibility
					render::text(
						{ p->bbox.x + p->bbox.w * 0.5f + 1, p->bbox.y - 21 + 1 },
						ImColor(10, 10, 10, 255),
						render::FONT_CENTER_X | render::FONT_CENTER_Y,
						buf
					);

					// Actual team ID text
					render::text(
						{ p->bbox.x + p->bbox.w * 0.5f, p->bbox.y - 21 },
						distanceColor,
						render::FONT_CENTER_X | render::FONT_CENTER_Y,
						buf
					);
				}

				if (showHealthESP)
				{
					const auto factor = static_cast<float>(p->player_name.health) / 127.f;
					const float hue = (factor * 120.f) / 360.f;

					rect({ p->bbox.x - 5 - 1, p->bbox.y - 1 }, { 2 + 2, p->bbox.z + 2 }, IM_COL32(10, 10, 10, 255));
					rect({ p->bbox.x - 5 - 1 + 1, p->bbox.y - 1 + 1 }, { 2 - 2, p->bbox.z - 2 }, IM_COL32(10, 10, 10, 255));
					rect_filled({ p->bbox.x - 5, p->bbox.y }, { 2, p->bbox.z }, { 50, 50, 50 });
					rect_filled({ p->bbox.x - 5, p->bbox.y + p->bbox.z * (1.f - factor) }, { 2, p->bbox.z - p->bbox.z * (1.f - factor) }, col_t::from_hsb(hue, 1.f, 1.f));
				}

				if (showBoneESP)
				{
					ImGui::GetWindowDrawList()->AddLine(ImVec2(p->bone_array.j_neckpos_screen.x, p->bone_array.j_neckpos_screen.y), ImVec2(p->bone_array.j_spineupperpos_screen.x, p->bone_array.j_spineupperpos_screen.y), boneColor, game_data->settings.boldness);
					ImGui::GetWindowDrawList()->AddLine(ImVec2(p->bone_array.j_spineupperpos_screen.x, p->bone_array.j_spineupperpos_screen.y), ImVec2(p->bone_array.j_spinelowerpos_screen.x, p->bone_array.j_spinelowerpos_screen.y), boneColor, game_data->settings.boldness);
					ImGui::GetWindowDrawList()->AddLine(ImVec2(p->bone_array.j_neckpos_screen.x, p->bone_array.j_neckpos_screen.y), ImVec2(p->bone_array.j_clavicle_ripos_screen.x, p->bone_array.j_clavicle_ripos_screen.y), boneColor, game_data->settings.boldness);
					ImGui::GetWindowDrawList()->AddLine(ImVec2(p->bone_array.j_clavicle_ripos_screen.x, p->bone_array.j_clavicle_ripos_screen.y), ImVec2(p->bone_array.j_shoulder_ripos_screen.x, p->bone_array.j_shoulder_ripos_screen.y), boneColor, game_data->settings.boldness);
					ImGui::GetWindowDrawList()->AddLine(ImVec2(p->bone_array.j_shoulder_ripos_screen.x, p->bone_array.j_shoulder_ripos_screen.y), ImVec2(p->bone_array.j_elbow_ripos_screen.x, p->bone_array.j_elbow_ripos_screen.y), boneColor, game_data->settings.boldness);
					ImGui::GetWindowDrawList()->AddLine(ImVec2(p->bone_array.j_elbow_ripos_screen.x, p->bone_array.j_elbow_ripos_screen.y), ImVec2(p->bone_array.j_wrist_ripos_screen.x, p->bone_array.j_wrist_ripos_screen.y), boneColor, game_data->settings.boldness);
					ImGui::GetWindowDrawList()->AddLine(ImVec2(p->bone_array.j_neckpos_screen.x, p->bone_array.j_neckpos_screen.y), ImVec2(p->bone_array.j_clavicle_lepos_screen.x, p->bone_array.j_clavicle_lepos_screen.y), boneColor, game_data->settings.boldness);
					ImGui::GetWindowDrawList()->AddLine(ImVec2(p->bone_array.j_clavicle_lepos_screen.x, p->bone_array.j_clavicle_lepos_screen.y), ImVec2(p->bone_array.j_shoulder_lepos_screen.x, p->bone_array.j_shoulder_lepos_screen.y), boneColor, game_data->settings.boldness);
					ImGui::GetWindowDrawList()->AddLine(ImVec2(p->bone_array.j_shoulder_lepos_screen.x, p->bone_array.j_shoulder_lepos_screen.y), ImVec2(p->bone_array.j_elbow_lepos_screen.x, p->bone_array.j_elbow_lepos_screen.y), boneColor, game_data->settings.boldness);
					ImGui::GetWindowDrawList()->AddLine(ImVec2(p->bone_array.j_elbow_lepos_screen.x, p->bone_array.j_elbow_lepos_screen.y), ImVec2(p->bone_array.j_wrist_lepos_screen.x, p->bone_array.j_wrist_lepos_screen.y), boneColor, game_data->settings.boldness);
					ImGui::GetWindowDrawList()->AddLine(ImVec2(p->bone_array.j_spinelowerpos_screen.x, p->bone_array.j_spinelowerpos_screen.y), ImVec2(p->bone_array.j_mainrootpos_screen.x, p->bone_array.j_mainrootpos_screen.y), boneColor, game_data->settings.boldness);
					ImGui::GetWindowDrawList()->AddLine(ImVec2(p->bone_array.j_spinelowerpos_screen.x, p->bone_array.j_spinelowerpos_screen.y), ImVec2(p->bone_array.j_mainrootpos_screen.x, p->bone_array.j_mainrootpos_screen.y), boneColor, game_data->settings.boldness);
					ImGui::GetWindowDrawList()->AddLine(ImVec2(p->bone_array.j_mainrootpos_screen.x, p->bone_array.j_mainrootpos_screen.y), ImVec2(p->bone_array.j_hip_lepos_screen.x, p->bone_array.j_hip_lepos_screen.y), boneColor, game_data->settings.boldness);
					ImGui::GetWindowDrawList()->AddLine(ImVec2(p->bone_array.j_mainrootpos_screen.x, p->bone_array.j_mainrootpos_screen.y), ImVec2(p->bone_array.j_hip_ripos_screen.x, p->bone_array.j_hip_ripos_screen.y), boneColor, game_data->settings.boldness);
					ImGui::GetWindowDrawList()->AddLine(ImVec2(p->bone_array.j_hip_ripos_screen.x, p->bone_array.j_hip_ripos_screen.y), ImVec2(p->bone_array.j_knee_ripos_screen.x, p->bone_array.j_knee_ripos_screen.y), boneColor, game_data->settings.boldness);
					ImGui::GetWindowDrawList()->AddLine(ImVec2(p->bone_array.j_hip_lepos_screen.x, p->bone_array.j_hip_lepos_screen.y), ImVec2(p->bone_array.j_knee_lepos_screen.x, p->bone_array.j_knee_lepos_screen.y), boneColor, game_data->settings.boldness);
					ImGui::GetWindowDrawList()->AddLine(ImVec2(p->bone_array.j_knee_ripos_screen.x, p->bone_array.j_knee_ripos_screen.y), ImVec2(p->bone_array.j_ankle_ripos_screen.x, p->bone_array.j_ankle_ripos_screen.y), boneColor, game_data->settings.boldness);
					ImGui::GetWindowDrawList()->AddLine(ImVec2(p->bone_array.j_knee_lepos_screen.x, p->bone_array.j_knee_lepos_screen.y), ImVec2(p->bone_array.j_ankle_lepos_screen.x, p->bone_array.j_ankle_lepos_screen.y), boneColor, game_data->settings.boldness);
					ImGui::GetWindowDrawList()->AddLine(ImVec2(p->bone_array.j_ankle_ripos_screen.x, p->bone_array.j_ankle_ripos_screen.y), ImVec2(p->bone_array.j_ball_ripos_screen.x, p->bone_array.j_ball_ripos_screen.y), boneColor, game_data->settings.boldness);
					ImGui::GetWindowDrawList()->AddLine(ImVec2(p->bone_array.j_ankle_lepos_screen.x, p->bone_array.j_ankle_lepos_screen.y), ImVec2(p->bone_array.j_ball_lepos_screen.x, p->bone_array.j_ball_lepos_screen.y), boneColor, game_data->settings.boldness);
				}

				if (showHeadESP)
				{
					// Calculate the horizontal and vertical distances between helmet and neck positions
					float dx = p->bone_array.j_helmetpos_screen.x - p->bone_array.j_neckpos_screen.x;
					float dy = p->bone_array.j_helmetpos_screen.y - p->bone_array.j_neckpos_screen.y;

					// Use std::hypot to calculate the Euclidean distance more accurately and safely
					float distance = std::hypot(dx, dy);

					// Compute the radius as half the distance
					float radius = distance / 2;

					// Retrieve the window draw list from ImGui and add a circle at the computed radius
					ImVec2 headPosition(p->bone_array.real_head_pos_screen.x, p->bone_array.real_head_pos_screen.y);
					ImGui::GetWindowDrawList()->AddCircle(headPosition, radius, boneColor, 100, 1.0f);
				}
				//}
			}

			/*if (game_data->settings.compass_esp)
			{
				ImGui::SetNextWindowSize(ImVec2(flRadarSize, flRadarSize + ImGui::GetFrameHeight()));
				ImGui::Begin(XOR("R"), &game_data->settings.compass_esp, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
				ImGui::GetWindowDrawList()->PushClipRectFullScreen();
				vRadarPosition[0] = ImGui::GetWindowPos().x + ImGui::GetWindowSize().x / 2.0f;
				vRadarPosition[1] = (ImGui::GetWindowPos().y + ImGui::GetFrameHeight()) + (ImGui::GetWindowSize().y - ImGui::GetFrameHeight()) / 2.0f;
				ImGui::GetWindowDrawList()->AddLine(ImVec2(vRadarPosition[0] - flRadarSize / 2.0f + 1.0f, vRadarPosition[1]),
					ImVec2(vRadarPosition[0] + flRadarSize / 2.0f - 1.0f, vRadarPosition[1]), ImGui::GetColorU32(ImVec4(0.50f, 0.50f, 0.50f, 0.50f)));

				ImGui::GetWindowDrawList()->AddLine(ImVec2(vRadarPosition[0], vRadarPosition[1] - flRadarSize / 2.0f),
					ImVec2(vRadarPosition[0], vRadarPosition[1] + flRadarSize / 2.0f - 1.0f), ImGui::GetColorU32(ImVec4(0.50f, 0.50f, 0.50f, 0.50f)));

				ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(vRadarPosition[0] - 2.0f, vRadarPosition[1] - 2.0f),
					ImVec2(vRadarPosition[0] + 3.0f, vRadarPosition[1] + 3.0f), ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)));
				for (size_t i = 0; i <= 2046; i++)
				{
					player* p = &players[i];
					if (p->is_valid)
					{
						if (p->distance > game_data->settings.distanceFloat)
							continue;
						if (p->team == players[client::localPlayer->clientNum].team && game_data->settings.enemies_only)
							continue;
						ImVec4 color{};

						color = ImVec4(255.f / 255, 0.f, 0.f, 255.f / 4);


						ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(vBlipPosition[i][0], vBlipPosition[i][1]), flBlipSize / 2.0f,
							ImGui::GetColorU32(color));

						ImGui::GetWindowDrawList()->AddCircle(ImVec2(vBlipPosition[i][0], vBlipPosition[i][1]), flBlipSize / 2.0f,
							ImGui::GetColorU32(color));
					}
				}

				ImGui::End();
			}*/

			if (game_data->settings.realCompass)
			{
				if (p->distance < game_data->settings.distanceFloat && p->distance > 0.5f)
				{
					if (game_data->settings.enemies_only)
						if (p->team == players[client::localPlayer->clientNum].team)
							continue;

					if (game_data->settings.show_only_visible)
						if (!p->visible)
							continue;

					if (game_data->settings.aim_target_esp && bestTarget != nullptr && p == bestTarget)
					{
						game_data->settings.compasscolor = ImGui::ColorConvertFloat4ToU32(game_data->settings.Target_color);
					}
					else if (game_data->settings.show_aim_warning_nigs && p->isLookingAtMe)
					{
						game_data->settings.compasscolor = ImGui::ColorConvertFloat4ToU32(game_data->settings.V2_COMPASS_colorVec4);
					}
					else if (game_data->settings.b_visible && p->visible)
					{
						game_data->settings.compasscolor = ImGui::ColorConvertFloat4ToU32(game_data->settings.V_COMPASS_colorVec4);
					}
					else if (game_data->settings.Compass_rgb)
					{
						game_data->settings.compasscolor = game_data->settings.chamColorRGB;
					}
					else
					{
						game_data->settings.compasscolor = ImGui::ColorConvertFloat4ToU32(game_data->settings.COMPASS_colorVec4);
					}

					auto base_x = ImVec2(game_data->settings.arrowSize / -2.0f, 0.0f);
					auto base_y = ImVec2(game_data->settings.arrowSize / 2.0f, game_data->settings.arrowSize / 2.0f * -0.75f);
					auto base_z = ImVec2(game_data->settings.arrowSize / 2.0f, game_data->settings.arrowSize / 2.0f * +0.75f);
					auto angle = arrowAngle[i];
					auto final_x = AddVecs(arrow_pos[i], ImRotate(base_x, ImCos(angle), ImSin(angle)));
					auto final_y = AddVecs(arrow_pos[i], ImRotate(base_y, ImCos(angle), ImSin(angle)));
					auto final_z = AddVecs(arrow_pos[i], ImRotate(base_z, ImCos(angle), ImSin(angle)));

					// Draw the triangle outline in black
					ImGui::GetWindowDrawList()->AddLine(final_x, final_y, game_data->settings.compasscolor - ImColor(0, 0, 0, 20), 3.0f);
					ImGui::GetWindowDrawList()->AddLine(final_y, final_z, game_data->settings.compasscolor - ImColor(0, 0, 0, 20), 3.0f);
					ImGui::GetWindowDrawList()->AddLine(final_z, final_x, game_data->settings.compasscolor - ImColor(0, 0, 0, 20), 3.0f);

					// Draw the filled triangle with the specified color
					ImGui::GetWindowDrawList()->AddTriangleFilled(final_x, final_y, final_z, game_data->settings.compasscolor - ImColor(0, 0, 0, static_cast<int>(game_data->settings.arrowfill)));
				}
			}
		}
	}
}

void DrawLineWithX(ImVec2 shootPos, ImVec2 hitPos, ImU32 col, float lineThickness, float xSize) {
	ImDrawList* drawList = ImGui::GetWindowDrawList();

	// Set the color for the line (e.g., white) and draw the line
	drawList->AddLine(shootPos, hitPos, col, lineThickness);

	// Draw the "X" at the hit position
	// Calculate the endpoints of the "X"
	ImVec2 xStart1(hitPos.x - xSize, hitPos.y - xSize);
	ImVec2 xEnd1(hitPos.x + xSize, hitPos.y + xSize);
	ImVec2 xStart2(hitPos.x - xSize, hitPos.y + xSize);
	ImVec2 xEnd2(hitPos.x + xSize, hitPos.y - xSize);

	// Draw the two lines of the "X"
	drawList->AddLine(xStart1, xEnd1, col, lineThickness); // Red color for "X"
	drawList->AddLine(xStart2, xEnd2, col, lineThickness); // Red color for "X"
}

void DrawTracers()
{

	for (const auto& tracer : tracers)
	{
		if (tracer.opacity > 0.0f)
		{
			ImU32 tracerColor = game_data->settings.rgb_bullettracer
				? ImGui::GetColorU32(ImVec4(tempRainbow[0] / 255.0f, tempRainbow[1] / 255.0f, tempRainbow[2] / 255.0f, 1.0f))
				: ImGui::ColorConvertFloat4ToU32(game_data->settings.bullettracer_color);

			DrawLineWithX(
				ImVec2(tracer.start2D.x, tracer.start2D.y),
				ImVec2(tracer.hit2D.x, tracer.hit2D.y),
				tracerColor,
				game_data->settings.bullet_trace_thickness,
				game_data->settings.bullet_x_trace_thickness
			);

			const_cast<cTracer&>(tracer).opacity -= game_data->settings.bullet_trace_fade;
		}
	}

	// Remove tracers with zero opacity
	tracers.erase(std::remove_if(tracers.begin(), tracers.end(),
		[](const cTracer& tracer) { return tracer.opacity <= 0.0f; }),
		tracers.end());


}

namespace g_game
{

	struct HookInfo
	{
		bool& hookFlag;
		LPVOID hookFunction;
	};

	void hookcheck()
	{
		game_data->settings.b_ingame = in_game();
		if (!game_data->settings.b_ingame)
		{
			game_data->settings.checkPassed = false;

			HookInfo hooks[] = {
				//{game_data->settings.CL_InputMP_ReadyToSendPacket_hooked, (LPVOID*)(game_data->game_offsets.CL_InputMP_ReadyToSendPacket)},
				//{game_data->settings.cg_hooked, (LPVOID*)(game_data->game_offsets.cg_t)},
				//{game_data->settings.DamageDirectionIndicators_hooked, (LPVOID*)(game_data->game_offsets.CG_DrawDamageDirectionIndicators)},
				{game_data->settings.PredictPlayerState_hooked, (LPVOID*)(game_data->game_offsets.PredictPlayerState)},
				{game_data->settings.no_recoilhooked, (LPVOID*)(game_data->game_offsets.BG_WeaponFireRecoilOffset)},
				{game_data->settings.no_spreadhooked, (LPVOID*)(game_data->game_offsets.BG_CalculateFinalSpreadForWeapon)},
				{game_data->settings.shellshockhooked, (LPVOID*)(game_data->game_offsets.CG_StartShellShock)},
				{game_data->settings.chamHooked, (LPVOID*)(game_data->game_offsets.R_AddDObjToScene)},
				{game_data->settings.HUDchamHooked, (LPVOID*)(game_data->game_offsets.R_AddViewmodelDObjToScene)},
				//{game_data->settings.CL_hooked, (LPVOID*)(game_data->game_offsets.A_CL_Input_ClearAutoForwardFlag)},
				//{game_data->settings.fovhooked, (LPVOID*)(game_data->game_offsets.fov_hook)},
				{game_data->settings.thirdpersonhook, (LPVOID*)(game_data->game_offsets.A_BG_ThirdPerson_IsEnabled)},
				{game_data->settings.oCG_EntityMP_CalcLerpPositions_hook, (LPVOID*)(game_data->game_offsets.CG_EntityMP_CalcLerpPositions)},
				{game_data->settings.uavtest_hooked, (LPVOID*)(game_data->game_offsets.A_DrawMapLayer)},
				{game_data->settings.bullet_tracers_hooked, (LPVOID*)(game_data->game_offsets.a_BulletHitEvent_Internal)},
			};

			for (const HookInfo& hook : hooks)
			{
				if (hook.hookFlag)
				{
					MH_DisableHook(hook.hookFunction);
					MH_RemoveHook(hook.hookFunction);
					hook.hookFlag = false;
				}
			}
		}

		init_hooks();

		if (GetAsyncKeyState(VK_F1) & 1) {
			LoadSettings(1);
		}

		if (GetAsyncKeyState(VK_F2) & 1) {
			LoadSettings(2);
		}

		if (GetAsyncKeyState(VK_F3) & 1) {
			LoadSettings(3);
		}

		if (GetAsyncKeyState(VK_F4) & 1) {
			LoadSettings(4);
		}

		if (GetAsyncKeyState(VK_F5) & 1) {
			LoadSettings(5);
		}


		if (game_data->settings.spamchat)
		{
			Spam_message(currentMessage);
		}
	}

	void main_loop(ImFont* font)
	{
		ESPThread();
		DrawTracers();
		updateRainbowColor();
		game_data->settings.chamColorRGB = ImGui::GetColorU32(ImVec4((float)tempRainbow[0] / 255, (float)tempRainbow[1] / 255, (float)tempRainbow[2] / 255, 255 / 255));
		if (game_data->settings.RGB_Menu)
		{
			main_color = game_data->settings.chamColorRGB;
		}

		LootThread();
	}

	void ui_header()
	{
		ImGuiIO& io = ImGui::GetIO();

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		ImGui::Begin(XOR("A"), reinterpret_cast<bool*>(true), ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);
		ImGui::SetWindowPos(ImVec2(0, 0)); // Only set once
		ImGui::SetWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y)); // Only set once
	}

	void ui_end()
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		window->DrawList->PushClipRectFullScreen();
		ImGui::End();
		ImGui::PopStyleColor();
	}

	void init(ImFont* font)
	{
		ui_header();
		main_loop(font);
		ui_end();
	}

}