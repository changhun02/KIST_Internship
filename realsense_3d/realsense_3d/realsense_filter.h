#pragma once
#include "stdafx.h"

class filter_options
{
public:
	filter_options(const std::string name, rs2::filter& filter);
	filter_options(filter_options&& other);
	std::string filter_name;                                   //Friendly name of the filter
	rs2::filter& filter;                                       //The filter in use
	std::map<rs2_option, filter_slider_ui> supported_options;  //maps from an option supported by the filter, to the corresponding slider
	std::atomic_bool is_enabled;                               //A boolean controlled by the user that determines whether to apply the filter or not
};
