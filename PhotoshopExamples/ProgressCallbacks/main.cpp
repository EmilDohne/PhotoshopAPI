/*
Example of writing a large PhotoshopFile asynchronously while using a callback to continuously query the state and progress of the file read operation
WARNING: This will write a rather large file to your disk ~1GB

This example does not have a python counterpart as we do not have an equivalent counterpart for the ProgressCallback& in python, if you would like 
to suggest a change that would implement this please do so by creating a github pull request.
*/

#include "PhotoshopAPI.h"

#include <unordered_map>
#include <vector>
#include <random>
#include <ctime>
#include <iostream>

#include <future>
#include <functional>
#include <chrono>
#include <thread>


using namespace NAMESPACE_PSAPI;


// Simple wrapper function which simply executes the write asynchronously, this could also just be a lambda 
template <typename T>
void AsyncWriteFile(LayeredFile<T>&& document, std::filesystem::path filePath, ProgressCallback& callback)
{
	LayeredFile<T>::write(std::move(document), filePath, callback);
}


int main()
{
	uint32_t width = 4096;
	uint32_t height = 4096;
	auto document = LayeredFile<bpp32_t>(Enum::ColorMode::RGB, width, height);

	// Generate some random image data which will compress badly to emulate longer write times
	std::mt19937 rng(static_cast<unsigned>(std::time(0)));
	std::uniform_real_distribution<bpp32_t> dist(0.0, 1.0);
	std::vector<bpp32_t> imgData(width * height);
	for (auto& value : imgData)
	{
		value = dist(rng);
	}
	// Add the layer 5 times to make execution longer
	for (int i = 0; i < 5; ++i)
	{
		std::unordered_map <Enum::ChannelID, std::vector<bpp32_t>> channelMap;
		channelMap[Enum::ChannelID::Red] = imgData;
		channelMap[Enum::ChannelID::Green] = imgData;
		channelMap[Enum::ChannelID::Blue] = imgData;
		ImageLayer<bpp32_t>::Params layerParams = {};
		layerParams.name = "Layer_" + std::to_string(i);
		layerParams.width = width;
		layerParams.height = height;

		auto layer = std::make_shared<ImageLayer<bpp32_t>>(std::move(channelMap), layerParams);
		document.add_layer(layer);
	}

	// Launch the file read asynchronously while attaching a callback which we pass by reference
	ProgressCallback callback{};
	auto future = std::async(std::launch::async,
		&AsyncWriteFile<bpp32_t>, std::move(document), "ProgressCallbackExample.psd", std::ref(callback));

	// Simulate some kind of loop where we continuously query the state of the progress until done
	while (true)
	{
		if (future.valid())
			if (future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
			{
				std::cout << "Writing of file " << callback.getProgress() * 100 << "% completed. Current task: " << callback.getTask() << std::endl;
				break;
			}
		std::cout << "Writing of file " << callback.getProgress()*100 << "% completed. Current task: " << callback.getTask() << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
	std::cout << "Finished writing file: 'ProgressCallbackExample.psd'" << std::endl;
}
