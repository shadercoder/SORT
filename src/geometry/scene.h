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

#ifndef	SORT_SCENE
#define	SORT_SCENE

// include the header file
#include "sort.h"
#include <vector>
#include "trimesh.h"
#include "spectrum/spectrum.h"
#include "thirdparty/tinyxml/tinyxml.h"

// pre-decleration of classes
class Accelerator;
class Light;
class Distribution1D;
class TiXmlNode;

////////////////////////////////////////////////////////////////////////////
// definition of scene class
class	Scene
{
// public method
public:
	// destructor
	~Scene(){ Release(); }

	// load the scene from script file
	// para 'root' : the node for the file
	// result     : 'true' if parsing is successful
	bool	LoadScene(  TiXmlNode* root );

	// get the intersection between a ray and the scene
	// para 'r' : the ray
	// result   : the intersection information between the ray and the scene
	// note     : if there is no acceleration structure , it will iterator all
	//			  of the triangles which will cost much!
	bool	GetIntersect( const Ray& r , Intersection* intersect ) const;

	// release the memory of the scene
	void	Release();

	// preprocess
	void	PreProcess();

	// get light
	const Light* GetLight( unsigned i ) const
	{
		sAssert( i < m_lights.size() , LIGHT );
		return m_lights[i];
	}
	// get lights
	const vector<Light*>& GetLights() const
	{return m_lights;}
	// get sky light
	const Light* GetSkyLight() const
	{return m_skyLight;}
	// get sampled light
	const Light* SampleLight( float u , float* pdf ) const;
	// get the properbility of the sample
	float LightProperbility( unsigned i ) const;
	// get the number of lights
	unsigned LightNum() const
	{ return (unsigned)m_lights.size(); }
	// get bounding box of the scene
	const BBox& GetBBox() const;
	// get triangle mesh with a specific name
	TriMesh*	GetTriMesh( const string& name ) const;

	// get file name
	const string& GetFileName() const
	{ return m_filename; }

	// Evaluate sky
	Spectrum	Le( const Ray& ray ) const;

// private field
private:
	// the buffer for the triangle mesh
	vector<TriMesh*>	m_meshBuf;

	// the triangle buffer for the scene
	vector<Primitive*>	m_triBuf;

	// the light
	vector<Light*>		m_lights;
	// distribution of light power
	Distribution1D*		m_pLightsDis = nullptr;
	// the sky light
	Light*				m_skyLight = nullptr;

	// the acceleration structure for the scene
	Accelerator*		m_pAccelerator = nullptr;

	// the file name for the scene
	string		m_filename;

	// bounding box for the scene
	mutable BBox	m_BBox;

// private method
	
	// brute force intersection test ( it will only invoked if there is no acceleration structor
	// para 'r' : the ray
	// result   : the intersection information between the ray and the scene
	bool	_bfIntersect( const Ray& r , Intersection* intersect ) const;

	// generate triangle buffer
	void	_generateTriBuf();

	// initialize default data
	void	_init();

	// parse transformation
	Transform	_parseTransform( const TiXmlElement* node );

	// compute light cdf
	void	_genLightDistribution();
};

#endif
