// clang-format off

/*
 * Custom module import
 *
 * Default SWIG module import extended by instructions to ensure that the shared sensorring
 * library can be located and loaded on Windows. This is necessary because since Python 3.8
 * the directories listed in the PATH environment variable are no longer searched.
 *
 *  https://bugs.python.org/issue43173
 */
%define MODULEIMPORT
"
import platform
import sys
import os

# Ensure that the sensorring.dll location is added to the Python DLL search path. This is
# necessary because since Python 3.8 the directories listed in the PATH variable are no 
# longer searched.
if sys.version_info >= (3, 8) and platform.system() == 'Windows':
    install_dir = os.environ.get('EDU_SENSORRING_DIR')
    if install_dir == None:
        # Use relative location as fallback
        module_dir = os.path.dirname(__file__)
        os.add_dll_directory(os.path.join(module_dir, '../../../bin'))
    else:
        os.add_dll_directory(os.path.join(install_dir, 'bin'))

# Import the low-level C/C++ module
if __package__ or '.' in __name__:
    from . import $module
else:
    import $module
"
%enddef


/*
 * Module name
 *
 * moduleimport: Custom import logic for the C/C++ module.
 * directors   : Activates director support for seamless polymorphism with the the target language.
 * threads     : Thread support for the Python wrapper. Required for multithreaded C/C++ library but reduces
 *               the performance of the wrapper.
 * package     : Python package name.
 */
%module (moduleimport=MODULEIMPORT, directors="1", threads="1", package="eduart") sensorring


// The following headers are included in the generated wrapper code
%{
#define SWIG_FILE_WITH_INIT

#include "sensorring/logger/Logger.hpp"
#include "sensorring/logger/LoggerTypes.hpp"
#include "sensorring/logger/LoggerClient.hpp"
#include "sensorring/platform/SensorringExport.hpp"
#include "sensorring/types/Image.hpp"
#include "sensorring/types/LightMode.hpp"
#include "sensorring/types/InterfaceType.hpp"
#include "sensorring/types/PointCloud.hpp"
#include "sensorring/types/TofMeasurement.hpp"
#include "sensorring/types/ThermalMeasurement.hpp"
#include "sensorring/types/SubscriberToken.hpp"
#include "sensorring/types/Subscription.hpp"
#include "sensorring/math/Math.hpp"
#include "sensorring/math/Vector3.hpp"
#include "sensorring/math/Matrix3.hpp"
#include "sensorring/device/DeviceType.hpp"
#include "sensorring/manager/ManagerState.hpp"
#include "sensorring/manager/MeasurementClient.hpp"
#include "sensorring/manager/MeasurementManager.hpp"
#include "sensorring/Parameter.hpp"
%}


// Include additional SWIG functionality
%include <stdint.i>
%include <exception.i>
%include <std_except.i>
%include <std_array.i>
%include <std_string.i>
%include <std_vector.i>


// NumPy support
%include "numpy.i"
%init %{
  import_array();
%}


namespace std {
// Aliases for integer types
typedef ::uint8_t uint8_t;
typedef ::uint16_t uint16_t;
typedef ::uint32_t uint32_t;
typedef ::uint64_t uint64_t;
typedef ::int8_t int8_t;
typedef ::int16_t int16_t;
typedef ::int32_t int32_t;
typedef ::int64_t int64_t;
} // namespace std


// Ignore functionality that is too C/C++ specific
// Operators
%ignore* ::operator<<;
%ignore* ::operator();
%ignore* ::operator==;
%ignore* ::operator!=;
%ignore* ::operator<;
%ignore* ::operator=;


// Iterators
%ignore* ::begin;
%ignore* ::end;


// Type mappings for methods coping data to NumPy
%apply (double*  INPLACE_ARRAY_FLAT, int DIM_FLAT) {(double*  buffer, int size)};


// Type mappings for methods coping data to NumPy
//%apply (unsigned char*  INPLACE_ARRAY_FLAT, int DIM_FLAT) {(unsigned char*  destination, int size)};
//%apply (unsigned short* INPLACE_ARRAY_FLAT, int DIM_FLAT) {(unsigned short* destination, int size)};
//%apply (float*          INPLACE_ARRAY_FLAT, int DIM_FLAT) {(float*          destination, int size)};

/*
 * The following section defines the C++ symbols to be made available in the target language.
 *
 * Be careful! The sequence of the instructions below matters! Dependencies have to be included before they are used.
 *
 * %include - Include all the declared symbol and wrap them
 * %import  - Parse the type information but do not wrap the declared symbols
 *
 * %apply    - Applies a special type mapping
 * %clear    - Clears a special type mapping
 *
 * %catches  - Catches the specified exception from the named function and rethrows it in the target language
 *
 * %feature  - Activates a special feature
 *
 * %ignore   - Ignore the specified symbol
 * %rename   - Rename the specified symbol
 *
 * %template - Creates a wrapper with the given name for the specified template specialization
 */

/****
 * Commonly used definitions
 */

%import "sensorring/platform/SensorringExport.hpp"

%ignore eduart::math::Vector3::operator[];
%extend eduart::math::Vector3 {
    double __getitem__(int idx) {
        return $self->operator[](idx);
    }
    void __setitem__(int idx, double value) {
        $self->operator[](idx) = value;
    }
}
%template (VectorDataArray) std::array<double, 3>;
%include "sensorring/math/Vector3.hpp"


%ignore eduart::math::Matrix3::operator[];
%extend eduart::math::Matrix3 {
    eduart::math::Vector3& __getitem__(int idx) {
        return $self->operator[](idx);
    }
    void __setitem__(int idx, const eduart::math::Vector3 &value) {
        $self->operator[](idx) = value;
    }
}
%template (MatrixDataArray) std::array<eduart::math::Vector3, 3>;
%include "sensorring/math/Matrix3.hpp"


%include "sensorring/math/Math.hpp"


%include "sensorring/types/Image.hpp"


%include "sensorring/types/LightMode.hpp"


%include "sensorring/types/InterfaceType.hpp"


%include "sensorring/types/PointCloud.hpp"


%include "sensorring/types/SubscriberToken.hpp"


%include "sensorring/types/Subscription.hpp"


%template (PointDataVector) std::vector<eduart::measurement::PointData>;
%include "sensorring/types/TofMeasurement.hpp"

%template (TemperatureImageTemplate) eduart::measurement::GenericGrayscaleImage<std::uint8_t, eduart::THERMAL_RESOLUTION>;
%template (GrayscaleImageTemplate) eduart::measurement::GenericGrayscaleImage<double, eduart::THERMAL_RESOLUTION>;
%template (FalseColorImageTemplate) eduart::measurement::GenericRGBImage<std::uint8_t, eduart::THERMAL_RESOLUTION>;
%include "sensorring/types/ThermalMeasurement.hpp"


%typemap(in) std::chrono::milliseconds {
    if (PyLong_Check($input)) {
        long long v = PyLong_AsLongLong($input);
        $1 = std::chrono::milliseconds(v);
    } else {
        SWIG_exception_fail(SWIG_TypeError, "Expected integer for milliseconds");
    }
}
%typemap(out) std::chrono::milliseconds {
    $result = PyLong_FromLongLong($1.count());
}
%rename(timeout_ms) eduart::ring::RingParams::timeout;
%template (BusParamVector) std::vector<eduart::bus::BusParams>;
%template (BoardParamVector) std::vector<eduart::device::SensorBoardParams>;
%include "sensorring/Parameter.hpp"

%rename (ManagerStateToString) eduart::manager::toString(ManagerState);
%include "sensorring/device/DeviceType.hpp"
%include "sensorring/manager/ManagerState.hpp"

// --- MeasurementManager: SWIG cannot wrap std::unique_ptr. We ignore the C++ ctor
// and expose a factory that takes a raw pointer (ownership transferred from Python).
%ignore MeasurementManager(ManagerParams, std::unique_ptr<ring::SensorRing>);

%inline %{
namespace eduart { namespace manager {

  /** Factory for Python bindings: takes ownership of sensor_ring. */
  eduart::manager::MeasurementManager* make_MeasurementManager(eduart::manager::ManagerParams params, eduart::ring::SensorRing* sensor_ring) {
    return new eduart::manager::MeasurementManager(params, std::unique_ptr<eduart::ring::SensorRing>(sensor_ring));
  }

}}  // namespace eduart::manager
%}

%newobject eduart::manager::make_MeasurementManager(eduart::manager::ManagerParams, eduart::ring::SensorRing*);

// When passing a SensorRing into the factory, transfer ownership from Python to C++.
%typemap(in) eduart::ring::SensorRing* sensor_ring (int res = 0, void* argp = nullptr) {
  res = SWIG_ConvertPtr($input, &argp, $descriptor(eduart::ring::SensorRing*), SWIG_POINTER_DISOWN);
  if (!SWIG_IsOK(res)) {
    SWIG_exception_fail(SWIG_ArgError(res), "in method \"$symname\", argument $argnum of type \"eduart::ring::SensorRing *\" (ownership transferred)");
  }
  $1 = reinterpret_cast<eduart::ring::SensorRing*>(argp);
}

%exception eduart::manager::make_MeasurementManager {
    try {
        $action
    } catch (const std::exception& e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    }
}
%catches(std::runtime_error) eduart::manager::MeasurementManager::measureSome(const LogVerbosity, const std::string);
%include "sensorring/manager/MeasurementManager.hpp"

// Make MeasurementManager(params, sensor_ring) use our factory (same API as C++).
%pythoncode %{
def _MeasurementManager_init(self, params, sensor_ring):
    other = make_MeasurementManager(params, sensor_ring)
    self.this = other.this
    self.thisown = other.thisown
MeasurementManager.__init__ = _MeasurementManager_init
%}


%rename (LogVerbosityToString) toString(LogVerbosity);
%include "sensorring/logger/LoggerTypes.hpp"


%catches(std::runtime_error) eduart::logger::Logger::log(const LogVerbosity verbosity, const std::string& msg) const;
%ignore Logger::log(const LogVerbosity, const std::stringstream);
%include "sensorring/logger/Logger.hpp" 
