/*
  Copyright (c) 2021-2026 Magnus
  https://github.com/mp-se/gravitymon

  Licensed under the GNU General Public License v3.0 (GPL v3) for open source use,
  or a Commercial License for proprietary use. See LICENSE and LICENSE_COMMERCIAL.
*/


#ifndef SRC_PUSH_GRAVITYMON_HPP_
#define SRC_PUSH_GRAVITYMON_HPP_

#if defined(GRAVITYMON)

#include <config_gravitymon.hpp>
#include <templating.hpp>

void setupTemplateEngineGravity(GravitymonConfig* config,
                                TemplatingEngine& engine, float angle,
                                float velocity, float gravitySG,
                                float corrGravitySG, float tempC, float runTime,
                                float voltage);

#endif  // GRAVITYMON

#endif  // SRC_PUSH_GRAVITYMON_HPP_
