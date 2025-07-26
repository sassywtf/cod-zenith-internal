#pragma once
#include <algorithm>
#include <math.h>
#include <utility>

struct col_t
	{
		std::array<uint8_t, 4> m_color;

		col_t() = default;

		col_t(const int r, const int g, const int b) : m_color({ 0,0,0,0 })
		{
			set(r, g, b, 255);
		}

		col_t(const int r, const int g, const int b, const int a) : m_color({ 0,0,0,0 })
		{
			set(r, g, b, a);
		}

		col_t(const float r, const float g, const float b) : m_color({ 0,0,0,0 })
		{
			set(static_cast<int>(r * 255),
				static_cast<int>(g * 255),
				static_cast<int>(b * 255), 255);
		}

		col_t(const float r, const float g, const float b, const float a) : m_color({ 0,0,0,0 })
		{
			set(static_cast<int>(r * 255),
				static_cast<int>(g * 255),
				static_cast<int>(b * 255),
				static_cast<int>(a * 255));
		}

		col_t(const col_t& col, const int a) : m_color({ 0,0,0,0 })
		{
			set(col.r(), col.g(), col.b(), a);
		}

		inline void set(const int r, const int g, const int b, const int a)
		{
			m_color[0] = static_cast<uint8_t>(r);
			m_color[1] = static_cast<uint8_t>(g);
			m_color[2] = static_cast<uint8_t>(b);
			m_color[3] = static_cast<uint8_t>(a);
		}

		inline int r() const
		{
			return m_color[0];
		}

		inline int g() const
		{
			return m_color[1];
		}

		inline int b() const
		{
			return m_color[2];
		}

		inline int a() const
		{
			return m_color[3];
		}

		inline col_t& operator=(const col_t& in)
		{
			set(in.r(), in.g(), in.b(), in.a());
			return *this;
		}

		inline bool operator==(const col_t& in) const
		{
			return *const_cast<col_t*>(this) == *const_cast<col_t*>(&in);
		}

		inline bool operator!=(const col_t& in) const
		{
			return !(operator==(in));
		}

		struct palette_t
		{
			static col_t black(int alpha = 255) { return col_t(13, 13, 13, alpha); }
			static col_t white(int alpha = 255) { return col_t(220, 220, 220, alpha); }
			static col_t light_black(int alpha = 255) { return col_t(32, 32, 32, alpha); }
			static col_t dark_grey(int alpha = 255) { return col_t(44, 44, 44, alpha); }
			static col_t grey(int alpha = 255) { return col_t(60, 60, 60, alpha); }
			static col_t light_grey(int alpha = 255) { return col_t(77, 77, 77, alpha); }
			static col_t light_blue(int alpha = 255) { return col_t(100, 149, 237, alpha); }
			static col_t blue_violet(int alpha = 255) { return col_t(138, 43, 226, alpha); }
		};

		/// <summary>
		/// Creates a new color object from the given arguments
		/// </summary>
		/// <param name="hue">Hue of new color</param>
		/// <param name="saturation">Saturation of new color</param>
		/// <param name="brightness">Brightness of new color</param>
		/// <returns>New color object</returns>
		static col_t from_hsb(float hue, float saturation, float brightness)
		{
			float h = hue == 1.0f ? 0 : hue * 6.0f;
			float f = h - (int)h;
			float p = brightness * (1.0f - saturation);
			float q = brightness * (1.0f - saturation * f);
			float t = brightness * (1.0f - (saturation * (1.0f - f)));

			if (h < 1)
			{
				return col_t(
					(unsigned char)(brightness * 255),
					(unsigned char)(t * 255),
					(unsigned char)(p * 255)
				);
			}
			if (h < 2)
			{
				return col_t(
					(unsigned char)(q * 255),
					(unsigned char)(brightness * 255),
					(unsigned char)(p * 255)
				);
			}
			if (h < 3)
			{
				return col_t(
					(unsigned char)(p * 255),
					(unsigned char)(brightness * 255),
					(unsigned char)(t * 255)
				);
			}
			if (h < 4)
			{
				return col_t(
					(unsigned char)(p * 255),
					(unsigned char)(q * 255),
					(unsigned char)(brightness * 255)
				);
			}
			if (h < 5)
			{
				return col_t(
					(unsigned char)(t * 255),
					(unsigned char)(p * 255),
					(unsigned char)(brightness * 255)
				);
			}
			return col_t(
				(unsigned char)(brightness * 255),
				(unsigned char)(p * 255),
				(unsigned char)(q * 255)
			);
		}
	};