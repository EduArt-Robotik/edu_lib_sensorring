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
#include "sensorring/logger/LogVerbosity.hpp"
#include "sensorring/platform/SensorringExport.hpp"
#include "sensorring/measurement/Image.hpp"
#include "sensorring/device/light/LightMode.hpp"
#include "sensorring/measurement/PointCloud.hpp"
#include "sensorring/measurement/ThermalMeasurement.hpp"
#include "sensorring/subscription/SubscriberToken.hpp"
#include "sensorring/subscription/Subscription.hpp"
#include "sensorring/math/Math.hpp"
#include "sensorring/math/Vector3.hpp"
#include "sensorring/math/Matrix3.hpp"
#include "sensorring/interface/ComInterfaceID.hpp"
#include "sensorring/interface/InterfaceParams.hpp"
#include "sensorring/device/DeviceType.hpp"
#include "sensorring/device/types/DeviceID.hpp"
#include "sensorring/device/DeviceParams.hpp"
#include "sensorring/board/SensorBoardType.hpp"
#include "sensorring/device/depth/vl53l8cx/VL53L8CX_Params.hpp"
#include "sensorring/device/thermal/htpa32/HTPA32_Params.hpp"
#include "sensorring/device/light/ws2812b/WS2812b_Params.hpp"
#include "sensorring/device/depth/tmf8829/TMF8829_ResultFormat.hpp"
#include "sensorring/device/depth/tmf8829/TMF8829_Params.hpp"
#include "sensorring/board/SensorBoardParams.hpp"
#include "sensorring/measurement/Header.hpp"
#include "sensorring/device/types/DeviceState.hpp"
#include "sensorring/SensorBus.hpp"
#include "sensorring/SensorRing.hpp"
#include "sensorring/SensorRingFactory.hpp"
#include "sensorring/board/EnumerationInformation.hpp"
#include "sensorring/manager/ManagerParams.hpp"
#include "sensorring/manager/ManagerState.hpp"
#include "sensorring/manager/MeasurementManager.hpp"
#include "sensorring/device/action/Command.hpp"
#include "sensorring/device/action/ActionQueue.hpp"
#include "sensorring/device/action/ActionDispatcher.hpp"
#include "sensorring/device/Device.hpp"
#include "sensorring/device/Sensor.hpp"
#include "sensorring/device/depth/DepthSensorConfig.hpp"
#include "sensorring/device/depth/DepthSensorParams.hpp"
#include "sensorring/device/depth/DepthSensor.hpp"
#include "sensorring/device/thermal/ThermalSensorParams.hpp"
#include "sensorring/device/thermal/ThermalSensor.hpp"
#include "sensorring/device/light/LightParams.hpp"
#include "sensorring/device/light/Light.hpp"
#include "sensorring/device/types/Group.hpp"
#include "sensorring/measurement/DepthMeasurement.hpp"
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
%apply (unsigned char*  INPLACE_ARRAY_FLAT, int DIM_FLAT) {(unsigned char*  buffer, int size)};


// Type mappings for methods coping data to NumPy (unused)
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

%ignore eduart::sensorring::math::Vector3::operator[];
%extend eduart::sensorring::math::Vector3 {
    double __getitem__(int idx) {
        return $self->operator[](idx);
    }
    void __setitem__(int idx, double value) {
        $self->operator[](idx) = value;
    }
}
%template (VectorDataArray) std::array<double, 3>;

// Type mappings for Vector3
%typemap(out) double & {
    $result = PyFloat_FromDouble(*$1);
}

%typemap(out) const double & {
    $result = PyFloat_FromDouble(*$1);
}

%include "sensorring/math/Vector3.hpp"


%ignore eduart::sensorring::math::Matrix3::operator[];
%extend eduart::sensorring::math::Matrix3 {
    eduart::sensorring::math::Vector3& __getitem__(int idx) {
        return $self->operator[](idx);
    }
    void __setitem__(int idx, const eduart::sensorring::math::Vector3 &value) {
        $self->operator[](idx) = value;
    }
}
%template (MatrixDataArray) std::array<eduart::sensorring::math::Vector3, 3>;
%include "sensorring/math/Matrix3.hpp"


%include "sensorring/math/Math.hpp"


%rename (LogVerbosityToString) eduart::sensorring::logger::toString(LogVerbosity);
%include "sensorring/logger/LogVerbosity.hpp"


%include "sensorring/measurement/Image.hpp"


%include "sensorring/device/light/LightMode.hpp"


%rename (InterfaceTypeToString) eduart::sensorring::com::toString(InterfaceType);
%include "sensorring/interface/ComInterfaceID.hpp"


%include "sensorring/interface/InterfaceParams.hpp"


%include "sensorring/measurement/PointCloud.hpp"
%template (PointCloudVector) std::vector<eduart::sensorring::measurement::PointCloud>;


%include "sensorring/subscription/SubscriberToken.hpp"


%include "sensorring/subscription/Subscription.hpp"


%template (PointDataVector) std::vector<eduart::sensorring::measurement::PointData>;

%rename (DeviceStateToString) eduart::sensorring::device::toString(DeviceState);
%include "sensorring/device/types/DeviceState.hpp"
%include "sensorring/measurement/Header.hpp"

%template (ScalarImageUint8Template) eduart::sensorring::measurement::ScalarImage<std::uint8_t, eduart::sensorring::THERMAL_RESOLUTION>;
%template (ScalarImageDoubleTemplate) eduart::sensorring::measurement::ScalarImage<double, eduart::sensorring::THERMAL_RESOLUTION>;
%template (RgbImageTemplate) eduart::sensorring::measurement::RgbImage<std::uint8_t, eduart::sensorring::THERMAL_RESOLUTION>;
%include "sensorring/measurement/ThermalMeasurement.hpp"


// Typemap: std::chrono::system_clock::time_point <-> Python float (seconds since epoch)
%typemap(out) std::chrono::system_clock::time_point {
  auto epoch = std::chrono::duration_cast<std::chrono::microseconds>($1.time_since_epoch()).count();
  $result = PyFloat_FromDouble(static_cast<double>(epoch) / 1e6);
}
%typemap(in) std::chrono::system_clock::time_point {
  double secs = PyFloat_AsDouble($input);
  if (PyErr_Occurred()) SWIG_fail;
  auto us = static_cast<long long>(secs * 1e6);
  $1 = std::chrono::system_clock::time_point(std::chrono::microseconds(us));
}


%include "sensorring/measurement/DepthMeasurement.hpp"
%template (DepthMeasurementVector) std::vector<eduart::sensorring::measurement::DepthMeasurement>;




/****
 * Device type hierarchy
 */

%rename (DeviceTypeToString) eduart::sensorring::device::toString(DeviceType);
%include "sensorring/device/DeviceType.hpp"


%include "sensorring/device/types/DeviceID.hpp"


%rename (Orientation_None) eduart::sensorring::board::Orientation::None;
%include "sensorring/device/DeviceParams.hpp"


%include "sensorring/device/light/LightParams.hpp"


%include "sensorring/device/light/ws2812b/WS2812b_Params.hpp"


%include "sensorring/device/thermal/ThermalSensorParams.hpp"


%include "sensorring/device/thermal/htpa32/HTPA32_Params.hpp"


%include "sensorring/device/depth/DepthSensorParams.hpp"


%include "sensorring/device/depth/vl53l8cx/VL53L8CX_Params.hpp"


%rename (ResolutionModeToString) eduart::sensorring::device::toString(ResolutionMode);
%include "sensorring/device/depth/tmf8829/TMF8829_ResultFormat.hpp"
%include "sensorring/device/depth/tmf8829/TMF8829_Params.hpp"


%rename (SensorBoardTypeToString) eduart::sensorring::board::toString(SensorBoardType);
%include "sensorring/board/SensorBoardType.hpp"


%rename (OrientationToString) eduart::sensorring::board::toString(Orientation mode);
%include "sensorring/board/SensorBoardParams.hpp"


/****
 * Typed device interfaces (new API)
 */

// Device: action queue is internal, not for Python users
%import "sensorring/device/action/Command.hpp"
%import "sensorring/device/action/ActionQueue.hpp"
%include "sensorring/device/action/ActionDispatcher.hpp"
%include "sensorring/device/Device.hpp"

%ignore eduart::sensorring::device::Sensor::beginMeasurementWait;
%ignore eduart::sensorring::device::Sensor::beginDataAvailableWait;
%include "sensorring/device/Sensor.hpp"

// --- DepthSensor ---
%include "sensorring/device/depth/DepthSensorConfig.hpp"

%ignore eduart::sensorring::device::DepthSensor::subscribe;     // Manual GIL wrapper below
%ignore eduart::sensorring::device::DepthSensor::publishMeasurement;
%ignore eduart::sensorring::device::DepthSensor::_depth_publisher;
%include "sensorring/device/depth/DepthSensor.hpp"

// --- ThermalSensor ---
%ignore eduart::sensorring::device::ThermalSensor::subscribe;
%ignore eduart::sensorring::device::ThermalSensor::publishMeasurement;
%ignore eduart::sensorring::device::ThermalSensor::_thermal_publisher;
%include "sensorring/device/thermal/ThermalSensor.hpp"

// --- Light ---
%template(LightVector) std::vector<eduart::sensorring::device::Light*>;
%include "sensorring/device/light/Light.hpp"

// Dynamic casting helpers of the device types

%inline %{

#define DEFINE_DEVICE_CAST(Type)                     \
inline Type* as##Type(Device* d) {                   \
    return dynamic_cast<Type*>(d);                   \
}

namespace eduart::sensorring::device {

DEFINE_DEVICE_CAST(DepthSensor)
DEFINE_DEVICE_CAST(ThermalSensor)
DEFINE_DEVICE_CAST(Light)
//DEFINE_DEVICE_CAST(VL53L8CX_Device)
//DEFINE_DEVICE_CAST(TMF8829_Device)
//DEFINE_DEVICE_CAST(HTPA32_Device)

} // namespace eduart::sensorring::device
#undef DEFINE_DEVICE_CAST
%}

// --- Group<T> ---
%ignore eduart::sensorring::device::Group::subscribe;     // Manual GIL wrapper below
%ignore eduart::sensorring::device::Group::subscribeAll;  // Manual GIL wrapper below
%ignore eduart::sensorring::device::Group::begin;
%ignore eduart::sensorring::device::Group::end;
%ignore eduart::sensorring::device::Group::iterator;
%ignore eduart::sensorring::device::Group::const_iterator;

%ignore eduart::sensorring::device::Group::operator++;
%ignore eduart::sensorring::device::Group::operator[];
%include "sensorring/device/types/Group.hpp"

%template(DepthSensorGroup) eduart::sensorring::device::Group<eduart::sensorring::device::DepthSensor>;
%template(ThermalSensorGroup) eduart::sensorring::device::Group<eduart::sensorring::device::ThermalSensor>;
%template(LightGroup) eduart::sensorring::device::Group<eduart::sensorring::device::Light>;

%extend eduart::sensorring::device::Group<eduart::sensorring::device::DepthSensor> {
  eduart::sensorring::device::DepthSensor& __getitem__(int i) {
    if (i < 0) i += static_cast<int>($self->size());
    if (i < 0 || i >= static_cast<int>($self->size()))
      throw std::out_of_range("Group index out of range");
    return (*$self)[static_cast<std::size_t>(i)];
  }
  std::size_t __len__() { return $self->size(); }
}
%extend eduart::sensorring::device::Group<eduart::sensorring::device::ThermalSensor> {
  eduart::sensorring::device::ThermalSensor& __getitem__(int i) {
    if (i < 0) i += static_cast<int>($self->size());
    if (i < 0 || i >= static_cast<int>($self->size()))
      throw std::out_of_range("Group index out of range");
    return (*$self)[static_cast<std::size_t>(i)];
  }
  std::size_t __len__() { return $self->size(); }
}
%extend eduart::sensorring::device::Group<eduart::sensorring::device::Light> {
  eduart::sensorring::device::Light& __getitem__(int i) {
    if (i < 0) i += static_cast<int>($self->size());
    if (i < 0 || i >= static_cast<int>($self->size()))
      throw std::out_of_range("Group index out of range");
    return (*$self)[static_cast<std::size_t>(i)];
  }
  std::size_t __len__() { return $self->size(); }
}


/****
 * Manager parameters
 */

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
%rename(timeout_ms) eduart::sensorring::manager::ManagerParams::timeout;
%include "sensorring/manager/ManagerParams.hpp"


/****
 * Enumeration information
 */

%rename (ConnectionStateToString) eduart::sensorring::board::toString(ConnectionState);
%rename (ConfigurationStateToString) eduart::sensorring::board::toString(ConfigurationState);
%warnfilter(503) eduart::sensorring::Version;
%warnfilter(503) eduart::sensorring::CommitHash;
%template (DeviceTypeVector) std::vector<eduart::sensorring::device::DeviceType>;
%include "sensorring/board/EnumerationInformation.hpp"

%rename (ManagerStateToString) eduart::sensorring::manager::toString(ManagerState);
%include "sensorring/manager/ManagerState.hpp"


/****
 * SensorRing
 */
%ignore eduart::sensorring::SensorRing::SensorRing;
%template(DeviceVector) std::vector<eduart::sensorring::device::Device*>;
%import "sensorring/SensorBus.hpp"
%include "sensorring/SensorRing.hpp"


/****
 * SensorRingFactory
 */

// SWIG cannot handle std::unique_ptr natively.
// We ignore methods returning unique_ptr and provide alternatives.
%ignore eduart::sensorring::SensorRingFactory::EnumerationMap;
%ignore eduart::sensorring::SensorRingFactory::build;
%ignore eduart::sensorring::SensorRingFactory::enumerate;
%ignore eduart::sensorring::SensorRingFactory::getLatestEnumerationResult;
%include "sensorring/SensorRingFactory.hpp"

// Factory returning raw pointer from unique_ptr (ownership transferred to Python)
%inline %{
namespace eduart { namespace sensorring {

  eduart::sensorring::SensorRing* SensorRingFactory_build(eduart::sensorring::SensorRingFactory* factory) {
    auto ptr = factory->build();
    return ptr.release();
  }

  std::string SensorRingFactory_enumerate_str(eduart::sensorring::SensorRingFactory* factory) {
    factory->enumerate();
    return factory->printTopology();
  }

}}
%}
%newobject eduart::sensorring::SensorRingFactory_build;

%exception eduart::sensorring::SensorRingFactory_build {
    try {
        $action
    } catch (const std::exception& e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    }
}

// Attach factory wrappers as methods on the Python SensorRingFactory class
%pythoncode %{
def _SensorRingFactory_build(self):
    return SensorRingFactory_build(self)
def _SensorRingFactory_enumerate(self):
    return SensorRingFactory_enumerate_str(self)

SensorRingFactory.build = _SensorRingFactory_build
SensorRingFactory.enumerate = _SensorRingFactory_enumerate
%}

// --- MeasurementManager ---
// The factory constructor is directly wrappable (takes reference):
// MeasurementManager(ManagerParams, SensorRingFactory&)
// The unique_ptr<SensorRing> constructor is for C++ power users only — ignore in SWIG.
%ignore eduart::sensorring::manager::MeasurementManager::MeasurementManager(ManagerParams, std::unique_ptr<SensorRing>);
%ignore eduart::sensorring::manager::MeasurementManager::devices;
%ignore eduart::sensorring::manager::MeasurementManager::subscribeToStateChanges;
%include "sensorring/manager/MeasurementManager.hpp"


%rename (LogVerbosityToString) toString(LogVerbosity);
%include "sensorring/logger/LogVerbosity.hpp"


%catches(std::runtime_error) eduart::sensorring::logger::Logger::log(const LogVerbosity verbosity, const std::string& msg) const;
%ignore Logger::log(const LogVerbosity, const std::stringstream);
%ignore Logger::subscribe;
%ignore Logger::unsubscribe;
%include "sensorring/logger/Logger.hpp" 


/****
 * Function-based subscription helpers (Python callable -> std::function)
 *
 * These allow Python code to use manager.subscribeToStateChanges(callback) with
 * plain Python callables, mirroring the C++ lambda-based API.
 */

%{
#include <memory>

static eduart::sensorring::subscription::Subscription* Logger_subscribe_py(
    eduart::sensorring::logger::Logger* logger, PyObject* callable) {
  Py_INCREF(callable);
  auto prevent_leak = std::shared_ptr<PyObject>(callable, [](PyObject* p) {
    PyGILState_STATE gstate = PyGILState_Ensure();
    Py_DECREF(p);
    PyGILState_Release(gstate);
  });
  auto sub = logger->subscribe(
    [prevent_leak](const eduart::sensorring::logger::LogVerbosity verbosity, const std::string& msg) {
      PyGILState_STATE gstate = PyGILState_Ensure();
      PyObject* py_verb = PyLong_FromLong(static_cast<int>(verbosity));
      PyObject* py_msg = PyUnicode_FromStringAndSize(msg.c_str(), static_cast<Py_ssize_t>(msg.size()));
      PyObject* result = PyObject_CallFunctionObjArgs(prevent_leak.get(), py_verb, py_msg, nullptr);
      Py_XDECREF(py_verb);
      Py_XDECREF(py_msg);
      Py_XDECREF(result);
      if (PyErr_Occurred()) PyErr_Print();
      PyGILState_Release(gstate);
    }
  );
  return new eduart::sensorring::subscription::Subscription(std::move(sub));
}

static eduart::sensorring::subscription::Subscription* Manager_subscribeToStateChanges_py(
    eduart::sensorring::manager::MeasurementManager* mgr, PyObject* callable) {
  Py_INCREF(callable);
  auto prevent_leak = std::shared_ptr<PyObject>(callable, [](PyObject* p) {
    PyGILState_STATE gstate = PyGILState_Ensure();
    Py_DECREF(p);
    PyGILState_Release(gstate);
  });
  auto sub = mgr->subscribeToStateChanges(
    [prevent_leak](const eduart::sensorring::manager::ManagerState state) {
      PyGILState_STATE gstate = PyGILState_Ensure();
      PyObject* py_state = PyLong_FromLong(static_cast<int>(state));
      PyObject* result = PyObject_CallFunctionObjArgs(prevent_leak.get(), py_state, nullptr);
      Py_XDECREF(py_state);
      Py_XDECREF(result);
      if (PyErr_Occurred()) PyErr_Print();
      PyGILState_Release(gstate);
    }
  );
  return new eduart::sensorring::subscription::Subscription(std::move(sub));
}

%}

// Declare the helpers for SWIG to generate Python wrappers
%newobject Logger_subscribe_py;
%newobject Manager_subscribeToStateChanges_py;

eduart::sensorring::subscription::Subscription* Logger_subscribe_py(eduart::sensorring::logger::Logger* logger, PyObject* callable);
eduart::sensorring::subscription::Subscription* Manager_subscribeToStateChanges_py(eduart::sensorring::manager::MeasurementManager* mgr, PyObject* callable);

// Attach the subscribe helpers as methods on the Python wrapper classes
%pythoncode %{
def _Logger_subscribe(self, callback):
    """Subscribe to log messages with a Python callable: callback(verbosity, msg)."""
    return Logger_subscribe_py(self, callback)
Logger.subscribe = _Logger_subscribe

def _MeasurementManager_subscribeToStateChanges(self, callback):
    """Subscribe to state changes: callback(state)."""
    return Manager_subscribeToStateChanges_py(self, callback)
MeasurementManager.subscribeToStateChanges = _MeasurementManager_subscribeToStateChanges
%}


/****
 * Per-device subscribe helpers (new API)
 *
 * DepthSensor.subscribe(callback)      -> callback(DepthMeasurement)
 * ThermalSensor.subscribe(callback)    -> callback(ThermalMeasurement)
 * DepthSensorGroup.subscribe(callback) -> callback(DepthMeasurement)
 * ThermalSensorGroup.subscribe(callback) -> callback(ThermalMeasurement)
 */

%{

static eduart::sensorring::subscription::Subscription* DepthSensor_subscribe_py(
    eduart::sensorring::device::DepthSensor* sensor, PyObject* callable) {
  Py_INCREF(callable);
  auto prevent_leak = std::shared_ptr<PyObject>(callable, [](PyObject* p) {
    PyGILState_STATE gstate = PyGILState_Ensure();
    Py_DECREF(p);
    PyGILState_Release(gstate);
  });
  auto sub = sensor->subscribe(
    [prevent_leak](const eduart::sensorring::measurement::DepthMeasurement& meas) {
      PyGILState_STATE gstate = PyGILState_Ensure();
      swig_type_info* ti = SWIG_TypeQuery("eduart::sensorring::measurement::DepthMeasurement *");
      auto* copy = new eduart::sensorring::measurement::DepthMeasurement(meas);
      PyObject* py_meas = SWIG_NewPointerObj(copy, ti, SWIG_POINTER_OWN);
      PyObject* result = PyObject_CallFunctionObjArgs(prevent_leak.get(), py_meas, nullptr);
      Py_XDECREF(py_meas);
      Py_XDECREF(result);
      if (PyErr_Occurred()) PyErr_Print();
      PyGILState_Release(gstate);
    }
  );
  return new eduart::sensorring::subscription::Subscription(std::move(sub));
}

static eduart::sensorring::subscription::Subscription* ThermalSensor_subscribe_py(
    eduart::sensorring::device::ThermalSensor* sensor, PyObject* callable) {
  Py_INCREF(callable);
  auto prevent_leak = std::shared_ptr<PyObject>(callable, [](PyObject* p) {
    PyGILState_STATE gstate = PyGILState_Ensure();
    Py_DECREF(p);
    PyGILState_Release(gstate);
  });
  auto sub = sensor->subscribe(
    [prevent_leak](const eduart::sensorring::measurement::ThermalMeasurement& meas) {
      PyGILState_STATE gstate = PyGILState_Ensure();
      swig_type_info* ti = SWIG_TypeQuery("eduart::sensorring::measurement::ThermalMeasurement *");
      auto* copy = new eduart::sensorring::measurement::ThermalMeasurement(meas);
      PyObject* py_meas = SWIG_NewPointerObj(copy, ti, SWIG_POINTER_OWN);
      PyObject* result = PyObject_CallFunctionObjArgs(prevent_leak.get(), py_meas, nullptr);
      Py_XDECREF(py_meas);
      Py_XDECREF(result);
      if (PyErr_Occurred()) PyErr_Print();
      PyGILState_Release(gstate);
    }
  );
  return new eduart::sensorring::subscription::Subscription(std::move(sub));
}

static eduart::sensorring::subscription::Subscription* DepthSensorGroup_subscribe_py(
    eduart::sensorring::device::Group<eduart::sensorring::device::DepthSensor>* group, PyObject* callable) {
  Py_INCREF(callable);
  auto prevent_leak = std::shared_ptr<PyObject>(callable, [](PyObject* p) {
    PyGILState_STATE gstate = PyGILState_Ensure();
    Py_DECREF(p);
    PyGILState_Release(gstate);
  });
  auto cb = [prevent_leak](const eduart::sensorring::measurement::DepthMeasurement& meas) {
    PyGILState_STATE gstate = PyGILState_Ensure();
    swig_type_info* ti = SWIG_TypeQuery("eduart::sensorring::measurement::DepthMeasurement *");
    auto* copy = new eduart::sensorring::measurement::DepthMeasurement(meas);
    PyObject* py_meas = SWIG_NewPointerObj(copy, ti, SWIG_POINTER_OWN);
    PyObject* result = PyObject_CallFunctionObjArgs(prevent_leak.get(), py_meas, nullptr);
    Py_XDECREF(py_meas);
    Py_XDECREF(result);
    if (PyErr_Occurred()) PyErr_Print();
    PyGILState_Release(gstate);
  };
  auto sub = group->subscribe(cb);
  return new eduart::sensorring::subscription::Subscription(std::move(sub));
}

static eduart::sensorring::subscription::Subscription* ThermalSensorGroup_subscribe_py(
    eduart::sensorring::device::Group<eduart::sensorring::device::ThermalSensor>* group, PyObject* callable) {
  Py_INCREF(callable);
  auto prevent_leak = std::shared_ptr<PyObject>(callable, [](PyObject* p) {
    PyGILState_STATE gstate = PyGILState_Ensure();
    Py_DECREF(p);
    PyGILState_Release(gstate);
  });
  auto cb = [prevent_leak](const eduart::sensorring::measurement::ThermalMeasurement& meas) {
    PyGILState_STATE gstate = PyGILState_Ensure();
    swig_type_info* ti = SWIG_TypeQuery("eduart::sensorring::measurement::ThermalMeasurement *");
    auto* copy = new eduart::sensorring::measurement::ThermalMeasurement(meas);
    PyObject* py_meas = SWIG_NewPointerObj(copy, ti, SWIG_POINTER_OWN);
    PyObject* result = PyObject_CallFunctionObjArgs(prevent_leak.get(), py_meas, nullptr);
    Py_XDECREF(py_meas);
    Py_XDECREF(result);
    if (PyErr_Occurred()) PyErr_Print();
    PyGILState_Release(gstate);
  };
  auto sub = group->subscribe(cb);
  return new eduart::sensorring::subscription::Subscription(std::move(sub));
}

static eduart::sensorring::subscription::Subscription* DepthSensorGroup_subscribeAll_py(
    eduart::sensorring::device::Group<eduart::sensorring::device::DepthSensor>* group, PyObject* callable) {
  Py_INCREF(callable);
  auto prevent_leak = std::shared_ptr<PyObject>(callable, [](PyObject* p) {
    PyGILState_STATE gstate = PyGILState_Ensure();
    Py_DECREF(p);
    PyGILState_Release(gstate);
  });
  auto cb = [prevent_leak](const std::vector<eduart::sensorring::measurement::DepthMeasurement>& measurements) {
    PyGILState_STATE gstate = PyGILState_Ensure();
    swig_type_info* ti = SWIG_TypeQuery("eduart::sensorring::measurement::DepthMeasurement *");
    PyObject* py_list = PyList_New(static_cast<Py_ssize_t>(measurements.size()));
    for (std::size_t i = 0; i < measurements.size(); ++i) {
      auto* copy = new eduart::sensorring::measurement::DepthMeasurement(measurements[i]);
      PyObject* py_meas = SWIG_NewPointerObj(copy, ti, SWIG_POINTER_OWN);
      PyList_SET_ITEM(py_list, static_cast<Py_ssize_t>(i), py_meas);
    }
    PyObject* result = PyObject_CallFunctionObjArgs(prevent_leak.get(), py_list, nullptr);
    Py_XDECREF(py_list);
    Py_XDECREF(result);
    if (PyErr_Occurred()) PyErr_Print();
    PyGILState_Release(gstate);
  };
  auto sub = group->subscribeAll(cb);
  return new eduart::sensorring::subscription::Subscription(std::move(sub));
}

static eduart::sensorring::subscription::Subscription* ThermalSensorGroup_subscribeAll_py(
    eduart::sensorring::device::Group<eduart::sensorring::device::ThermalSensor>* group, PyObject* callable) {
  Py_INCREF(callable);
  auto prevent_leak = std::shared_ptr<PyObject>(callable, [](PyObject* p) {
    PyGILState_STATE gstate = PyGILState_Ensure();
    Py_DECREF(p);
    PyGILState_Release(gstate);
  });
  auto cb = [prevent_leak](const std::vector<eduart::sensorring::measurement::ThermalMeasurement>& measurements) {
    PyGILState_STATE gstate = PyGILState_Ensure();
    swig_type_info* ti = SWIG_TypeQuery("eduart::sensorring::measurement::ThermalMeasurement *");
    PyObject* py_list = PyList_New(static_cast<Py_ssize_t>(measurements.size()));
    for (std::size_t i = 0; i < measurements.size(); ++i) {
      auto* copy = new eduart::sensorring::measurement::ThermalMeasurement(measurements[i]);
      PyObject* py_meas = SWIG_NewPointerObj(copy, ti, SWIG_POINTER_OWN);
      PyList_SET_ITEM(py_list, static_cast<Py_ssize_t>(i), py_meas);
    }
    PyObject* result = PyObject_CallFunctionObjArgs(prevent_leak.get(), py_list, nullptr);
    Py_XDECREF(py_list);
    Py_XDECREF(result);
    if (PyErr_Occurred()) PyErr_Print();
    PyGILState_Release(gstate);
  };
  auto sub = group->subscribeAll(cb);
  return new eduart::sensorring::subscription::Subscription(std::move(sub));
}

%}

%newobject DepthSensor_subscribe_py;
%newobject ThermalSensor_subscribe_py;
%newobject DepthSensorGroup_subscribe_py;
%newobject ThermalSensorGroup_subscribe_py;
%newobject DepthSensorGroup_subscribeAll_py;
%newobject ThermalSensorGroup_subscribeAll_py;

eduart::sensorring::subscription::Subscription* DepthSensor_subscribe_py(eduart::sensorring::device::DepthSensor* sensor, PyObject* callable);
eduart::sensorring::subscription::Subscription* ThermalSensor_subscribe_py(eduart::sensorring::device::ThermalSensor* sensor, PyObject* callable);
eduart::sensorring::subscription::Subscription* DepthSensorGroup_subscribe_py(eduart::sensorring::device::Group<eduart::sensorring::device::DepthSensor>* group, PyObject* callable);
eduart::sensorring::subscription::Subscription* ThermalSensorGroup_subscribe_py(eduart::sensorring::device::Group<eduart::sensorring::device::ThermalSensor>* group, PyObject* callable);
eduart::sensorring::subscription::Subscription* DepthSensorGroup_subscribeAll_py(eduart::sensorring::device::Group<eduart::sensorring::device::DepthSensor>* group, PyObject* callable);
eduart::sensorring::subscription::Subscription* ThermalSensorGroup_subscribeAll_py(eduart::sensorring::device::Group<eduart::sensorring::device::ThermalSensor>* group, PyObject* callable);

%pythoncode %{
def _DepthSensor_subscribe(self, callback):
    """Subscribe to depth measurements: callback(DepthMeasurement)."""
    return DepthSensor_subscribe_py(self, callback)
DepthSensor.subscribe = _DepthSensor_subscribe

def _ThermalSensor_subscribe(self, callback):
    """Subscribe to thermal measurements: callback(ThermalMeasurement)."""
    return ThermalSensor_subscribe_py(self, callback)
ThermalSensor.subscribe = _ThermalSensor_subscribe

def _DepthSensorGroup_subscribe(self, callback):
    """Subscribe to depth measurements from all sensors in the group."""
    return DepthSensorGroup_subscribe_py(self, callback)
DepthSensorGroup.subscribe = _DepthSensorGroup_subscribe

def _ThermalSensorGroup_subscribe(self, callback):
    """Subscribe to thermal measurements from all sensors in the group."""
    return ThermalSensorGroup_subscribe_py(self, callback)
ThermalSensorGroup.subscribe = _ThermalSensorGroup_subscribe

def _DepthSensorGroup_subscribeAll(self, callback):
    """Subscribe to synchronized depth measurements from all sensors (one callback per complete frame)."""
    return DepthSensorGroup_subscribeAll_py(self, callback)
DepthSensorGroup.subscribeAll = _DepthSensorGroup_subscribeAll

def _ThermalSensorGroup_subscribeAll(self, callback):
    """Subscribe to synchronized thermal measurements from all sensors (one callback per complete frame)."""
    return ThermalSensorGroup_subscribeAll_py(self, callback)
ThermalSensorGroup.subscribeAll = _ThermalSensorGroup_subscribeAll
%}


/****
 * Client interfaces (director-enabled so Python classes can inherit and override)
 */

%template (ThermalMeasurementVector) std::vector<eduart::sensorring::measurement::ThermalMeasurement>;
