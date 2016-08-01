#ifndef HEADER_pa_Value2Days_hpp_ALREADY_INCLUDED
#define HEADER_pa_Value2Days_hpp_ALREADY_INCLUDED

#include "gubg/time/Armin.hpp"
#include "gubg/Strange.hpp"
#include "gubg/mss.hpp"

namespace pa { 
    class Value2Days
    {
        public:
            bool set(const std::string &str)
            {
                MSS_BEGIN(bool);
                gubg::Strange strange(str);
                if (strange.pop_if("armin"))
                {
                    armin_ = true;
                }
                else
                {
                    MSS(strange.pop_float(f_));
                }
                MSS_END();
            }
            bool convert(double &days, double value) const
            {
                MSS_BEGIN(bool);
                if (!armin_)
                {
                    days = value*f_;
                }
                else
                {
                    double minutes;
                    MSS(gubg::time::from_armin(minutes, value));
                    days = minutes/60.0/8.0;
                }
                MSS_END();
            }
        private:
            double f_ = 1.0;
            bool armin_ = false;
    };
} 

#endif
