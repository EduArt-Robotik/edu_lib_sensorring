#include "CustomTypes.hpp"
#include "Math.hpp"


namespace measurement{

// Explicit template instantiation for the used types
template class GenericGrayscaleImage<uint8_t, THERMAL_RESOLUTION>;
template class GenericGrayscaleImage<double, THERMAL_RESOLUTION>;

template<typename T, std::size_t RESOLUTION>
double GenericGrayscaleImage<T, RESOLUTION>::avg (){
	double result = 0;
	std::size_t i = 0;
	for(auto& element : data){
		result += static_cast<double>(element);
		i++;
	}
	result /= i;
	return result;
}

template<typename T, std::size_t RESOLUTION>
GenericGrayscaleImage<T, RESOLUTION>& GenericGrayscaleImage<T, RESOLUTION>::round (){
	for(auto& element : data){
		element = std::round(element);
	}
	return *this;
}

template<typename T, std::size_t RESOLUTION>
GenericGrayscaleImage<T, RESOLUTION>& GenericGrayscaleImage<T, RESOLUTION>::operator/= (const T other){
	std::size_t i = 0;
	for(auto& element : data){
		element =static_cast<double>(element) / static_cast<double>(other);
		i++;
	}
	return *this;
}

template<typename T, std::size_t RESOLUTION>
GenericGrayscaleImage<T, RESOLUTION>& GenericGrayscaleImage<T, RESOLUTION>::operator+= (const T other){
	std::size_t i = 0;
	for(auto& element : data){
		element += other;
		i++;
	}
	return *this;
}

template<typename T, std::size_t RESOLUTION>
GenericGrayscaleImage<T, RESOLUTION>& GenericGrayscaleImage<T, RESOLUTION>::operator+= (const GenericGrayscaleImage<T, RESOLUTION>& other){
	std::size_t i = 0;
	for(auto& element : data){
		element += other.data[i];
		i++;
	}
	return *this;
}

template<typename T, std::size_t RESOLUTION>
template<typename U>
GenericGrayscaleImage<T, RESOLUTION>& GenericGrayscaleImage<T, RESOLUTION>::operator+= (const GenericGrayscaleImage<U, RESOLUTION>& other) {
	for (std::size_t i = 0; i < RESOLUTION; ++i) {
		data[i] += static_cast<T>(other.data[i]);
	}
	return *this;
}

template<typename T, std::size_t RESOLUTION>
GenericGrayscaleImage<T, RESOLUTION>& GenericGrayscaleImage<T, RESOLUTION>::operator-= (const T other){
	std::size_t i = 0;
	for(auto& element : data){
		element -= other;
		i++;
	}
	return *this;
}

template<typename T, std::size_t RESOLUTION>
GenericGrayscaleImage<T, RESOLUTION>& GenericGrayscaleImage<T, RESOLUTION>::operator-= (const GenericGrayscaleImage<T, RESOLUTION>& other){
	std::size_t i = 0;
	for(auto& element : data){
		element -= other.data[i];
		i++;
	}
	return *this;
}

template<typename T, std::size_t RESOLUTION>
template<typename U>
GenericGrayscaleImage<T, RESOLUTION>& GenericGrayscaleImage<T, RESOLUTION>::operator-= (const GenericGrayscaleImage<U, RESOLUTION>& other) {
	for (std::size_t i = 0; i < RESOLUTION; ++i) {
		data[i] -= static_cast<T>(other.data[i]);
	}
	return *this;
}

void TofSensorMeasurement::reserve(std::size_t size){
        point_distance.reserve(size);
        point_sigma.reserve(size);
        point_data.reserve(size);
        point_data_transformed.reserve(size);
};


}; //namespace Measurement