#include "objects/teapot.hpp"

Teapot::Teapot()
    : collision(&teapot_mesh.transform)
    , teapot_mesh("assets/models/kettle.mdl"_asset)
{
	//collision.set_local_bounds();
}