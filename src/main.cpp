/*
 * Copyright (c) 2020-2024 Gustavo Valiente gustavo.valiente@protonmail.com
 * zlib License, see LICENSE file.
 */

#include "bn_core.h"
#include "bn_log.h"
#include <bn_fixed.h>
#include <bn_vector.h>
#include <bn_random.h>
#include "bn_display.h"
#include <bn_math.h>
#include <bn_keypad.h>
#include "bn_blending.h"
#include <bn_sprite_ptr.h>
#include <bn_camera_ptr.h>
#include <bn_sprite_animate_actions.h>
#include <bn_sprite_text_generator.h>
#include <bn_sprite_font.h>
#include "bn_sound_items.h"
#include <bn_string.h>
#include <bn_random.h>
#include <bn_math.h>
#include <bn_sound_actions.h>
#include <bn_cameras.h>
#include "bn_camera_actions.h"
#include <bn_optional.h>
#include <bn_blending.h>

#include "bn_music.h"
// #include "bn_music_items.h"
// #include "bn_music_items_info.h"

#include "bn_regular_bg_ptr.h"

#include "bn_sprite_items_score.h"
#include "bn_sprite_items_hiscore.h"
#include "bn_sprite_items_ship.h"
#include "bn_sprite_items_asteroid1.h"
#include "bn_sprite_items_items.h"
#include "bn_sprite_items_nick.h"
#include "bn_sprite_items_numbers.h"
#include "bn_sprite_items_lives.h"
#include "bn_regular_bg_items_starsbackground.h"
#include "bn_regular_bg_items_startscreen.h"
#include "bn_regular_bg_items_gameoverscreen.h"

using namespace bn;
using namespace core;
using namespace keypad;
using namespace sprite_items;
using namespace regular_bg_items;
using namespace sound_items;

#include "main.h"

enum ShipState
{
    state_playing,
    state_dead,
    state_wait,
    state_loading
};

int close(fixed_t<12> x1, fixed_t<12> x2, fixed_t<12> y1, fixed_t<12> y2, int threshold)
{
    return abs(x1 - x2) <= threshold && abs(y1 - y2) <= threshold;
}

int stage_title()
{

    return 0;
}

int update_vector_score(vector<sprite_ptr, 5> &vect, int value)
{

    int val[5];
    int divisor = 1;
    int max_ = 0;

    for (int i = 0; i < 5; i++)
    {
        val[i] = (value / divisor) % 10;
        divisor *= 10;
        if (value >= divisor)
        {
            max_ = i + 1;
        }
    }
    max_++;

    for (int t = 0; t < 5; t++)
    {
        vect.at(t) = numbers.create_sprite(vect.at(t).x(), vect.at(t).y(), (t < max_) ? val[t] + 1 : 0);
    }

    return 0;
}

int main()
{
    init();

    auto bg = starsbackground.create_bg(0, 64);
    bg.set_blending_enabled(true);
    blending::set_transparency_alpha(0);

    while (true)
    {
        for (int t = 0; t < 26; t++)
        {
            update();
        }

        fixed_t<12> alpha_level = 0;
        for (int t = 0; t < 8; t++)
        {
            blending::set_transparency_alpha(alpha_level);
            if (alpha_level < 1)
            {
                alpha_level += 0.125;
            }
            update();
        }
        bg.set_blending_enabled(false);

        for (int t = 0; t < 24; t++)
        {
            update();
        }

        sound_items::celebrate.play();

        auto spr_ship = ship.create_sprite(-76 + 6, 28 - 16, 0);
        spr_ship.set_scale(2, 2);

        int ticker = 0;

        {
            auto title = startscreen.create_bg(8, 64 - 16);
            auto spr_nick = nick.create_sprite(-73 + 8, 30 - 16, 0);

            while (ticker < 27 + 90)
            {
                ticker++;

                if (ticker > 27)
                {
                    title.set_visible(true);
                }

                update();
            }
        }

        sound_items::intro.play();

        while (spr_ship.horizontal_scale() > 1)
        {
            auto new_scale = spr_ship.horizontal_scale() - 0.0625;
            spr_ship.set_scale(new_scale, new_scale);
            update();
        }

        auto rnd = random();

        auto spr_score_label = score.create_sprite(-82, -67);
        auto spr_score_high_label = hiscore.create_sprite(84, -67);
        auto lives_label = lives.create_sprite(-101 - 2, 68 - 4);
        auto lives_label_text = lives.create_sprite(-101 + 16 - 2, 68 -4, 1);

        vector<sprite_ptr, 5> spr_score;
        vector<sprite_ptr, 5> spr_score_high;
        for (int t = 0; t < 5; t++)
        {
            auto new_local = numbers.create_sprite(-82 + 21 - (t * 8), -67 + 8, 1);
            auto new_high = numbers.create_sprite(84 + 21 - (t * 8), -67 + 8, 1);

            spr_score.push_back(new_local);
            spr_score_high.push_back(new_high);
        }

        // Define minimum horizontal spacing
        const int min_spacing = 100;     // Adjust as needed
        const int min_spacing_item = 75; // Adjust as needed
        int asteroid_rotation_speed[4];
        int initial_x_position = 300; // Starting x position off-screen

        vector<sprite_ptr, 6> spr_items;
        vector<sprite_ptr, 4> asteroids;

        for (int t = 0; t < 4; t++)
        {
            // Calculate x position ensuring minimum spacing
            int x_pos = initial_x_position + t * min_spacing;

            // Alternate y-position
            int y_multiplier = (t % 2 == 0) ? -1 : 1;
            int y_offset = 24 + rnd.get_int(32); // Random between 20 and 51
            int y_pos = y_offset * y_multiplier;

            auto asteroid = asteroid1.create_sprite(x_pos, y_pos, rnd.get_int(2));
            asteroid.set_scale(2, 2);
            asteroids.push_back(asteroid);

            // Assign random rotation speed between 1 and 7
            asteroid_rotation_speed[t] = rnd.get_int(24) + 1;
        }

        for (int t = 0; t < 6; t++)
        {
            // Calculate x position ensuring minimum spacing
            int x_pos = initial_x_position + t * min_spacing_item;

            // Alternate y-position
            int y_offset = -24 + rnd.get_int(64); // Random between 20 and 51
            int y_pos = y_offset;

            auto item = items.create_sprite(x_pos, y_pos, rnd.get_int(5));
            spr_items.push_back(item);
        }

        int whoosh = 0;
        int score = 0;
        int score_high = 0;
        int score_lives = 2;
        update_vector_score(spr_score, score);
        update_vector_score(spr_score_high, score_high);

        ShipState state = state_playing;
        int state_battery = 0;
        bool isPlaying = true;

        while (isPlaying)
        {
            if (state == state_loading)
            {
                bg.set_x(bg.x() - 2);
            }
            else
            {
                bg.set_x(bg.x() - 1);
            }

            bool moving = false;

            switch (state)
            {
            case state_playing:
            {
                state_battery = 64;

                if (spr_ship.x() > -90)
                {
                    spr_ship.set_x(spr_ship.x() - 2);
                }

                if (a_held())
                {
                    whoosh = 8;
                }

                if (whoosh > 0)
                {
                    moving = true;
                    spr_ship.set_x(spr_ship.x() + 3);
                    whoosh--;
                }

                if (up_held() && spr_ship.y() > -35)
                {
                    spr_ship.set_y(spr_ship.y() - 1);
                }
                else if (down_held() && spr_ship.y() < 35)
                {
                    spr_ship.set_y(spr_ship.y() + 1);
                };

                if (moving)
                {
                    spr_ship = ship.create_sprite(spr_ship.x(), spr_ship.y(), (ticker / 4) % 2);
                }
                else
                {
                    spr_ship = ship.create_sprite(spr_ship.x(), spr_ship.y(), 0);
                }

                break;
            }
            case state_dead:
            {

                spr_ship = ship.create_sprite(spr_ship.x(), spr_ship.y(), 2 + ((ticker / 8) % 2));

                if (state_battery > 0)
                {
                    state_battery--;
                }
                else
                {
                    state = state_wait;
                    state_battery = 53;
                }

                if (state_battery < 32)
                {
                    spr_items.clear();
                    asteroids.clear();
                }

                break;
            }
            case state_wait:
            {
                spr_ship.set_visible(false);

                if (state_battery > 0)
                {
                    state_battery--;
                }
                else
                {
                    state = state_loading;
                    spr_ship = ship.create_sprite(-160, 0, 0);

                    score_lives--;
                    if (score_lives > -1)
                    {
                        lives_label_text = lives.create_sprite(-101 + 16 - 2, 68 - 4, 3 - score_lives);
                    }

                    for (int t = 0; t < 4; t++)
                    {
                        // Calculate x position ensuring minimum spacing
                        int x_pos = initial_x_position + t * min_spacing;

                        // Alternate y-position
                        int y_multiplier = (t % 2 == 0) ? -1 : 1;
                        int y_offset = 24 + rnd.get_int(32); // Random between 20 and 51
                        int y_pos = y_offset * y_multiplier;

                        auto asteroid = asteroid1.create_sprite(x_pos, y_pos, rnd.get_int(2));
                        asteroid.set_scale(2, 2);
                        asteroids.push_back(asteroid);

                        // Assign random rotation speed between 1 and 7
                        asteroid_rotation_speed[t] = rnd.get_int(24) + 1;
                    }

                    for (int t = 0; t < 6; t++)
                    {
                        // Calculate x position ensuring minimum spacing
                        int x_pos = initial_x_position + t * min_spacing_item;

                        // Alternate y-position
                        int y_offset = -24 + rnd.get_int(64); // Random between 20 and 51
                        int y_pos = y_offset;

                        auto item = items.create_sprite(x_pos, y_pos, rnd.get_int(5));
                        spr_items.push_back(item);
                    }
                }

                break;
            }
            case state_loading:
            {
                BN_LOG("SCORE LIVES: ", score_lives, " - isPlaying: ", isPlaying);
                if (score_lives < 0)
                {
                    isPlaying = false;
                }
                else
                {
                    spr_ship.set_visible(true);
                    score = 0;

                    if (spr_ship.x() < 48)
                    {
                        spr_ship = ship.create_sprite(spr_ship.x() + 2, spr_ship.y(), (ticker / 4) % 2);
                    }
                    else
                    {
                        state = state_playing;
                    }
                }

                break;
            }
            default:
            {
                break;
            }
            }

            for (int t = 0; t < spr_items.size(); t++)
            {
                spr_items.at(t).set_x(spr_items.at(t).x() - 1);

                if (close(spr_items.at(t).x(), spr_ship.x(), spr_items.at(t).y(), spr_ship.y(), 16) && spr_items.at(t).visible() && state == state_playing)
                {
                    spr_items.at(t).set_visible(false);
                    sound_items::collect.play();

                    score += 200;
                    if (score_high < score)
                        score_high = score;

                    update_vector_score(spr_score, score);
                    update_vector_score(spr_score_high, score_high);
                }

                // Check if off-screen
                if (spr_items.at(t).x() < -152)
                {
                    // Find maximum x among other asteroids
                    int max_x = -152; // initial_x_position;
                    for (int i = 0; i < spr_items.size(); i++)
                    {
                        if (i != t && spr_items.at(i).x() > max_x)
                        {
                            max_x = spr_items.at(i).x().integer();
                        }
                    }
                    // Set new x position ensuring minimum spacing
                    int new_x = max_x + min_spacing_item;

                    // Alternate y-position
                    int new_y = -24 + rnd.get_int(64);

                    // Update asteroid position
                    spr_items.at(t).set_position(new_x, new_y);
                    spr_items.at(t).set_visible(true);
                }
            }

            for (int t = 0; t < asteroids.size(); t++)
            {
                // Update rotation
                asteroids.at(t).set_rotation_angle(((ticker) / asteroid_rotation_speed[t]) % 360);
                // Move left
                asteroids.at(t).set_x(asteroids.at(t).x() - 1);

                if (close(asteroids.at(t).x(), spr_ship.x(), asteroids.at(t).y(), spr_ship.y(), 28) && state == state_playing)
                {
                    state = state_dead;
                }

                // Check if off-screen
                if (asteroids.at(t).x() < -152)
                {
                    // Find maximum x among other asteroids
                    int max_x = -152; // initial_x_position;
                    for (int i = 0; i < asteroids.size(); i++)
                    {
                        if (i != t && asteroids.at(i).x() > max_x)
                        {
                            max_x = asteroids.at(i).x().integer();
                        }
                    }
                    // Set new x position ensuring minimum spacing
                    int new_x = max_x + min_spacing;

                    // Alternate y-position
                    int y_multiplier = (t % 2 == 0) ? -1 : 1;
                    int y_offset = 24 + rnd.get_int(32);
                    int new_y = y_offset * y_multiplier;

                    // Update asteroid position
                    asteroids.at(t).set_position(new_x, new_y);
                    // Assign new rotation speed
                    asteroid_rotation_speed[t] = rnd.get_int(24) + 1;
                }
            }

            ticker++;
            update();
        }

        auto bg2 = gameoverscreen.create_bg(8, 64 - 16);

        int play_mode = 0;
        while (play_mode == 0)
        {
            bg.set_x(bg.x() - 1);

            update();

            if (a_held())
            {
                play_mode = 1;
            }
            else if (b_held())
            {
                play_mode = 2;
            }
        }
    }

    return 0;
}