/*
   FileName:      perspective.cpp

   Created Time:  2011-08-04 12:52:08

   Auther:        Cao Jiayin

   Email:         soraytrace@hotmail.com

   Location:      China, Shanghai

   Description:   SORT is short for Simple Open-source Ray Tracing. Anyone could checkout the source code from
                'sourceforge', https://soraytrace.svn.sourceforge.net/svnroot/soraytrace. And anyone is free to
                modify or publish the source code. It's cross platform. You could compile the source code in 
                linux and windows , g++ or visual studio 2008 is required.
*/

// include the header file
#include "perspective.h"
#include "texture/rendertarget.h"
#include <math.h>
#include "utility/sassert.h"
#include "sampler/sample.h"
#include "utility/samplemethod.h"
#include "imagesensor/imagesensor.h"
#include "light/light.h"

IMPLEMENT_CREATOR( PerspectiveCamera );

void PerspectiveCamera::_init()
{
	// set camera type
	m_type = CT_PERSPECTIVE;
	
	// setup intial value for data
	m_lensRadius = 0.0f;	// by default , depth of field is not enabled
	m_fov = PI * 0.25f;
	
	_registerAllProperty();
}

// register all properties
void PerspectiveCamera::_registerAllProperty()
{
    _registerProperty( "aspect" , new AspectProperty( this ) );
    _registerProperty( "sensorsize" , new SensorSizeProperty( this ) );
    _registerProperty( "eye" , new EyeProperty( this ) );
    _registerProperty( "up" , new UpProperty( this ) );
    _registerProperty( "target" , new TargetProperty( this ) );
    _registerProperty( "fov" , new FovProperty( this ) );
    _registerProperty( "len" , new LenProperty( this ) );
}

// Preprocess
void PerspectiveCamera::PreProcess()
{
    float w = (float)m_imagesensor->GetWidth();
    float h = (float)m_imagesensor->GetHeight();
    float aspect = w/h * m_aspectRatioW/m_aspectRatioH;
    
    float yScale = 1.0f / tan( m_fov * 0.5f );
    float xScale = yScale / aspect;
    
    // handle different sensor size for blender
    m_imagePlaneDist = yScale * h * 0.5f;
    if( m_sensorW && m_sensorH )
    {
        if( m_aspectFit == 1 || ( m_aspectFit != 2 && aspect > 1 ))	// horizontal fit
        {
            xScale = 1.0f / tan( m_fov * 0.5f );
            yScale = xScale * aspect;
            m_imagePlaneDist = xScale * w * 0.5f;
        }else if( m_aspectFit == 2 || ( m_aspectFit != 1 && aspect < 1 ))	// vertical fit
        {
            yScale = 1.0f / tan( m_fov * 0.5f );
            xScale = yScale / aspect;
            m_imagePlaneDist = yScale * h * 0.5f;
        }
    }
    
    // update focal distance every time
    m_focalDistance = ( m_target - m_eye ).Length();
    m_forward = ( m_target - m_eye ) / m_focalDistance;
    
    // clip space to raster space
    m_clipToRaster = Scale( w , h , 1.0f ) * Scale( 0.5f , -0.5f , 1.0f ) * Translate( 1.0f , -1.0f , 0.0f ) ;
    m_cameraToClip = Perspective(xScale, yScale);
    m_cameraToRaster = m_clipToRaster * m_cameraToClip;
    m_worldToCamera = ViewLookat( m_eye , m_forward , m_up );
    m_worldToRaster = m_cameraToRaster * m_worldToCamera;
}

// generate ray
Ray	PerspectiveCamera::GenerateRay( unsigned pass_id , float x , float y , const PixelSample& ps ) const
{
	// check if there is render target
	Sort_Assert( m_imagesensor != 0 );
	
    const Point rastP( x + ps.img_u , y + ps.img_v , 0.0f );
    Vector view_dir = m_cameraToRaster.invMatrix( rastP );
	
	Ray r;
    r.m_Dir = view_dir.Normalize();
    
    // Handle DOF camera ray adaption
    if( m_lensRadius != 0 )
    {
        Point target = r(m_focalDistance / view_dir.z);
        
        float s , t;
		UniformSampleDisk( ps.dof_u , ps.dof_v , s , t );
        
        r.m_Ori.x = s * m_lensRadius;
        r.m_Ori.y = t * m_lensRadius;
        r.m_Dir = Normalize( target - r.m_Ori );
    }

    // transform the ray from camera space to world space
    r = m_worldToCamera.invMatrix( r );

	// calculate the pdf for camera ray
	const float cosAtCamera = Dot( m_forward , r.m_Dir );
	const float imagePointToCameraDist = m_imagePlaneDist / cosAtCamera;
	const float imageToSolidAngleFactor = imagePointToCameraDist * imagePointToCameraDist / cosAtCamera;

	// the pdf of the ray
	r.m_fPDF = imageToSolidAngleFactor;

	return r;
}

// get camera coordinate according to a view direction in world space
Vector2i PerspectiveCamera::GetScreenCoord(Point p, float* pdf, Visibility* visibility)
{
	Vector dir = Normalize(p - m_eye);

	// get view space dir
    Point rastP = m_worldToRaster( p );
    
	// Handle DOF camera ray adaption
	if( m_lensRadius != 0.0f )
	{
		Point view_target = m_worldToCamera( p );

		float s , t;
        UniformSampleDisk( sort_canonical() , sort_canonical() , s , t );

		Ray adapted_ray;
		adapted_ray.m_Ori = Point( s , t , 0.0f ) * m_lensRadius;
		adapted_ray.m_Dir = view_target - adapted_ray.m_Ori;
		adapted_ray.m_fMax = adapted_ray.m_Dir.Length();
		adapted_ray.m_Dir /= adapted_ray.m_fMax;
		adapted_ray.m_fMax -= 0.001f;
		adapted_ray.m_fMin += 0.001f;

		visibility->ray = m_worldToCamera.invMatrix( adapted_ray );

		Point view_focal_target = adapted_ray( m_focalDistance / adapted_ray.m_Dir.z );
		rastP = m_cameraToRaster( view_focal_target );
	}

	if( pdf )
	{
		// calculate the pdf for camera ray
		const float cosAtCamera = Dot( m_forward , dir );
		const float imagePointToCameraDist = m_imagePlaneDist / cosAtCamera;
		const float imageToSolidAngleFactor = imagePointToCameraDist * imagePointToCameraDist / cosAtCamera;

		// the pdf of the ray
		*pdf = imageToSolidAngleFactor ;
	}

	return Vector2i( rastP.x , rastP.y );
}