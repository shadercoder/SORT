/*
    This file is a part of SORT(Simple Open Ray Tracing), an open-source cross
    platform physically based renderer.
 
    Copyright (c) 2011-2018 by Cao Jiayin - All rights reserved.
 
    SORT is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 
    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#pragma once

#include "bxdf.h"

//! @brief SmoothCoat BRDF.
/**
 * 'Arbitrarily Layered Micro-Facet Surfaces'
 * http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.160.2363&rep=rep1&type=pdf
 *
 * This BRDF is a simplified version of the above mentioned work. Only one layer of coating is supported and the upper layer is a perfect smooth layer.
 */
class SmoothCoat : public Bxdf
{
public:
	//! Constructor
    //! @param basecolor        Direction-hemisphere reflection.
    //! @param thickness        Thickness of the layer.
    //! @param ior              Index of refraction outside the surface where the normal points to.
    //! @param weight           Weight of the BXDF.
    //! @param n                Normal from normal map.
    //! @param doubleSided      Whether the surface is double sided.
    SmoothCoat(const Spectrum& basecolor, const float thickness, const float ior, const Spectrum& weight, const Vector& n , bool doubleSided = false)
        : Bxdf(weight, (BXDF_TYPE)(BXDF_DIFFUSE | BXDF_REFLECTION), n, doubleSided) , basecolor(basecolor), thickness(thickness), ior(ior) {}
	
    //! Evaluate the BRDF
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @return     The Evaluated BRDF value.
    Spectrum f( const Vector& wo , const Vector& wi ) const override;
	
    //! @brief Importance sampling for the fresnel brdf.
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @param bs   Sample for bsdf that holds some random variables.
    //! @param pdf  Probability density of the selected direction.
    //! @return     The Evaluated BRDF value.
    Spectrum sample_f( const Vector& wo , Vector& wi , const BsdfSample& bs , float* pdf ) const override;
    
    //! @brief Evaluate the pdf of an existance direction given the Incident direction.
    //! @param wo   Exitant direction in shading coordinate.
    //! @param wi   Incident direction in shading coordinate.
    //! @return     The probability of choosing the out-going direction based on the Incident direction.
    float pdf( const Vector& wo , const Vector& wi ) const override;
    
private:
    const Spectrum basecolor ;  /**< Base color of the bottom layer. */
    const float thickness ;     /**< Thickness of the layer. */
    const float ior ;           /**< Index of refraction out side the surface where the normal points. */
};