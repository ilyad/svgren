#pragma once

#include <array>
#include <vector>
#include <cstdint>
#include <stdexcept>

#include <utki/config.hpp>

#include <r4/matrix2.hpp>
#include <r4/rectangle.hpp>

#include <svgdom/elements/styleable.hpp>

#include "config.hxx"
#include "surface.hxx"
#include "gradient.hxx"

#if SVGREN_BACKEND == SVGREN_BACKEND_CAIRO
#	if M_OS == M_OS_WINDOWS || M_OS_NAME == M_OS_NAME_IOS
#		include <cairo.h>
#	else
#		include <cairo/cairo.h>
#	endif
#endif

namespace svgren{

inline r4::vector4<unsigned> get_rgba(uint32_t c){
#if SVGREN_BACKEND == SVGREN_BACKEND_CAIRO
	// cairo uses BGRA format
	return {
		unsigned((c >> 16) & 0xff),
		unsigned((c >> 8) & 0xff),
		unsigned((c >> 0) & 0xff),
		unsigned((c >> 24) & 0xff)
	};
#endif
}

inline uint32_t get_uint32_t(const r4::vector4<unsigned>& rgba){
#if SVGREN_BACKEND == SVGREN_BACKEND_CAIRO
	// cairo uses BGRA format
	return rgba.b() | (rgba.g() << 8) | (rgba.r() << 16) | (rgba.a() << 24);
#endif
}

class canvas{
	std::vector<uint32_t> data;

#if SVGREN_BACKEND == SVGREN_BACKEND_CAIRO
	struct cairo_surface_wrapper{
		cairo_surface_t* surface;

		cairo_surface_wrapper(unsigned width, unsigned height, uint32_t* buffer){
			if(width == 0 || height == 0){
				throw std::invalid_argument("svgren::canvas::canvas(): width or height argument is zero");
			}
			int stride = width * sizeof(uint32_t);
			this->surface = cairo_image_surface_create_for_data(
				reinterpret_cast<unsigned char*>(buffer),
				CAIRO_FORMAT_ARGB32,
				width,
				height,
				stride
			);
			if(!this->surface){
				throw std::runtime_error("svgren::canvas::canvas(): could not create cairo surface");
			}
		}
		~cairo_surface_wrapper(){
			cairo_surface_destroy(this->surface);
		}
	} surface;

	cairo_t* cr;
#endif

public:

	canvas(unsigned width, unsigned height);
	~canvas();

	void transform(const r4::matrix2<real>& matrix);
	void translate(real x, real y);
	void translate(const r4::vector2<real>& v){
		this->translate(v.x(), v.y());
	}
	void rotate(real radians);
	void scale(real x, real y);
	void scale(const r4::vector2<real>& v){
		this->scale(v.x(), v.y());
	}

	enum class fill_rule{
		even_odd,
		winding
	};

	void set_fill_rule(fill_rule fr);

	void set_source(const r4::vector4<real>& rgba);
	void set_source(const linear_gradient& g);
	void set_source(const radial_gradient& g);

	r4::vector2<real> matrix_mul(const r4::vector2<real>& v);

	// multiply by current matrix without translation part
	r4::vector2<real> matrix_mul_distance(const r4::vector2<real>& v);

	r4::rectangle<real> get_shape_bounding_box()const;

	bool has_current_point()const;
	r4::vector2<real> get_current_point()const;

	void move_to_abs(const r4::vector2<real>& p);
	void move_to_rel(const r4::vector2<real>& p);

	void line_to_abs(const r4::vector2<real>& p);
	void line_to_rel(const r4::vector2<real>& p);

	void quadratic_curve_to_abs(const r4::vector2<real>& cp1, const r4::vector2<real>& ep);
	void quadratic_curve_to_rel(const r4::vector2<real>& cp1, const r4::vector2<real>& ep);

	void cubic_curve_to_abs(const r4::vector2<real>& cp1, const r4::vector2<real>& cp2, const r4::vector2<real>& ep);
	void cubic_curve_to_rel(const r4::vector2<real>& cp1, const r4::vector2<real>& cp2, const r4::vector2<real>& ep);

	// draw arc from angle1 to angle2
	void arc_abs(const r4::vector2<real>& center, real radius, real angle1, real angle2);

	void close_path();

	void clear_path();

	void fill();
	void stroke();

	void set_line_width(real width);
	void set_line_cap(svgdom::stroke_line_cap lc);
	void set_line_join(svgdom::stroke_line_join lj);

	void rectangle(const r4::rectangle<real>& rect);

	void push_context();
	void pop_context();

	r4::matrix2<real> get_matrix();
	void set_matrix(const r4::matrix2<real>& m);

	void push_group();
	void pop_group(real opacity);
	void pop_mask_and_group();

	svgren::surface get_sub_surface(const r4::rectangle<unsigned>& region = {0, std::numeric_limits<unsigned>::max()});

	std::vector<uint32_t> release();
};

}
