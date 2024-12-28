/*
Example of loading a PhotoshopFile with a SmartObject layer, replacing the linked image (while keeping the warp) and creating a new layer with the image layer.
*/
#include "PhotoshopAPI.h"
#include "Core/Geometry/MeshOperations.h"
#include <Eigen/Dense>


int main()
{
	using namespace NAMESPACE_PSAPI;

	// Reading the file works as normal, as well as accessing the layers.
	LayeredFile<bpp8_t> file = LayeredFile<bpp8_t>::read("SmartObject.psd");
	auto layer_ptr = find_layer_as<bpp8_t, SmartObjectLayer>("warped", file);

	// If we want to add an additional smart object layer we can do this just like you would with a normal layer.
	// The only exception is that we have to provide a path for the image file we wish to link. We can additionally provide a warp
	// object which will reapply the given warp to the new layer. This is convenient if you wish to create a copy of a layer without
	// removing the other layer
	auto params = Layer<bpp8_t>::Params{};
	params.name = "uv_grid";
	auto layer_new = std::make_shared<SmartObjectLayer<bpp8_t>>(file, params, "uv_grid.jpg", layer_ptr->warp(), LinkedLayerType::external);

	// Smart objects also have more freedom in their transformations which we expose in the API:
	layer_new->rotate(45);	// Rotate by 45 degrees
	layer_new->scale(.65f);	// Scale by a factor of .65
	layer_new->move(Geometry::Point2D<double>(50.0f, 0.0f));	// Move to the right by 50

	//// We can additionally also apply a perspective transformation using a 3x3 projective matrix.
	//// This could also be used to skew or to directly apply all the above operations. This matrix e.g.
	//// applies a perspective transform where the vanishing point is at 2048, 0 (keeping in mind that 0 
	//// in this case is at the top of the canvas). 
	//// If you wish to know more about how these 3x3 transformation matrices work this is a great video/channel:
	//// https://www.youtube.com/watch?v=B8kMB6Hv2eI.
	Eigen::Matrix3d transformation;
	transformation << 1, 0, 0,
					  0, 1, 0,
					  1.0f / 2048, 0, 1;
	layer_new->transform(transformation);

	// If you want to be a bit more descriptive of how these matrices are to be built you can create one using a source
	// and destination quad like this. Keep in mind that these have to be in the coordinate space of the image itself:
	std::array<Geometry::Point2D<double>, 4> source_transform = Geometry::create_quad<double>(2048, 2048);
	std::array<Geometry::Point2D<double>, 4> destination_transform = Geometry::create_quad<double>(2048, 2048);
	// Squash together the top of the image
	destination_transform[0].x = 512;
	destination_transform[1].x = 1536;

	// Create a 3x3 homography mapping the source_transform quad to the destination_transform quad.
	auto homography = Geometry::Operations::create_homography_matrix(source_transform, destination_transform);
	layer_new->transform(homography);

	// If now we wanted to clear the transformation and/or warp that could be done as well. This will now just
	// be a smart object layer with width and height of 2048 (like the original image data).
	layer_new->reset_transform();
	layer_new->reset_warp();

	// For modifying the warp directly (bezier) we can push the individual points, although this is a bit more
	// advanced and we don't currently have a very convenient interface for it. This code right here 
	// pushes the top left corner to the 50, 50 position which will create a slightly rounded corner
	auto& warp = layer_new->warp();
	warp.point(0, 0).x = 50.0f;
	warp.point(0, 0).y = 50.0f;

	// adding these works just as with any other layers, writing also works as usual
	file.add_layer(layer_new);
	LayeredFile<bpp8_t>::write(std::move(file), "SmartObject_out.psb");
}