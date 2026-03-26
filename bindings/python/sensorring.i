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
#include "sensorring/types/PointCloud.hpp"
#include "sensorring/types/TofMeasurement.hpp"
#include "sensorring/types/ThermalMeasurement.hpp"
#include "sensorring/types/SubscriberToken.hpp"
#include "sensorring/types/Subscription.hpp"
#include "sensorring/math/Math.hpp"
#include "sensorring/math/Vector3.hpp"
#include "sensorring/math/Matrix3.hpp"
#include "sensorring/interface/ComInterfaceID.hpp"
#include "sensorring/device/DeviceType.hpp"
#include "sensorring/device/DeviceID.hpp"
#include "sensorring/device/DeviceParams.hpp"
#include "sensorring/device/hardware/SensorBoardType.hpp"
#include "sensorring/device/hardware/vl53l8cx/VL53L8CX_Params.hpp"
#include "sensorring/device/hardware/htpa32/HTPA32_Params.hpp"
#include "sensorring/device/hardware/ws2812b/WS2812b_Params.hpp"
#include "sensorring/SensorBoardParams.hpp"
#include "sensorring/SensorRing.hpp"
#include "sensorring/SensorRingFactory.hpp"
#include "sensorring/enumeration/EnumerationInformation.hpp"
#include "sensorring/manager/ManagerParams.hpp"
#include "sensorring/manager/ManagerState.hpp"
#include "sensorring/manager/MeasurementClient.hpp"
#include "sensorring/manager/MeasurementManager.hpp"
#include "sensorring/device/IDevice.hpp"
#include "sensorring/device/DeviceGroup.hpp"
#include "sensorring/device/hardware/ws2812b/WS2812b_Device.hpp"
#include "sensorring/device/hardware/vl53l8cx/VL53L8CX_Device.hpp"
#include "sensorring/device/hardware/htpa32/HTPA32_Device.hpp"
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


%include "sensorring/interface/ComInterfaceID.hpp"


%include "sensorring/types/PointCloud.hpp"


%include "sensorring/types/SubscriberToken.hpp"


%include "sensorring/types/Subscription.hpp"


%template (PointDataVector) std::vector<eduart::measurement::PointData>;
%include "sensorring/types/TofMeasurement.hpp"

%template (TemperatureImageTemplate) eduart::measurement::GenericGrayscaleImage<std::uint8_t, eduart::THERMAL_RESOLUTION>;
%template (GrayscaleImageTemplate) eduart::measurement::GenericGrayscaleImage<double, eduart::THERMAL_RESOLUTION>;
%template (FalseColorImageTemplate) eduart::measurement::GenericRGBImage<std::uint8_t, eduart::THERMAL_RESOLUTION>;
%include "sensorring/types/ThermalMeasurement.hpp"


/****
 * Device type hierarchy
 */

%rename (DeviceTypeToString) eduart::device::toString(DeviceType);
%include "sensorring/device/DeviceType.hpp"


%include "sensorring/device/DeviceID.hpp"


%include "sensorring/device/DeviceParams.hpp"


%include "sensorring/device/hardware/ws2812b/WS2812b_Params.hpp"


%include "sensorring/device/hardware/vl53l8cx/VL53L8CX_Params.hpp"


%include "sensorring/device/hardware/htpa32/HTPA32_Params.hpp"


%rename (SensorBoardTypeToString) eduart::device::toString(SensorBoardType);
%include "sensorring/device/hardware/SensorBoardType.hpp"


%include "sensorring/SensorBoardParams.hpp"


/****
 * Device group (read-only wrapper for subscription callbacks)
 */

%import "sensorring/device/IDevice.hpp"
%ignore eduart::device::DeviceGroup::DeviceGroup;
%ignore eduart::device::DeviceGroup::getDevices;
%ignore eduart::device::DeviceGroup::invokeForEachDevice;
%ignore eduart::device::DeviceGroup::getDevicesOfType;
%ignore eduart::device::DeviceGroup::invokeForEachDeviceOfType;
%ignore eduart::device::DeviceGroup::createFromDevicesOfType;
%ignore eduart::device::DeviceGroup::waitForAll;
%include "sensorring/device/DeviceGroup.hpp"


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
%rename(timeout_ms) eduart::manager::ManagerParams::timeout;
%include "sensorring/manager/ManagerParams.hpp"


/****
 * Enumeration information
 */

%rename (ConnectionStateToString) eduart::device::toString(ConnectionState);
%rename (ConfigurationStateToString) eduart::device::toString(ConfigurationState);
%template (DeviceTypeVector) std::vector<eduart::device::DeviceType>;
%include "sensorring/enumeration/EnumerationInformation.hpp"

%rename (ManagerStateToString) eduart::manager::toString(ManagerState);
%include "sensorring/manager/ManagerState.hpp"


/****
 * SensorRingFactory
 */

// SWIG cannot handle std::variant or std::unique_ptr natively.
// We ignore the C++ methods that use them and provide typed alternatives.
%ignore eduart::ring::SensorRingFactory::DeviceParamsVariant;
%ignore eduart::ring::SensorRingFactory::EnumerationMap;
%ignore eduart::ring::SensorRingFactory::build;
%ignore eduart::ring::SensorRingFactory::enumerate;
%ignore eduart::ring::SensorRingFactory::getLatestEnumerationResult;
%ignore eduart::ring::SensorRingFactory::expectBoard(device::SensorBoardParams, std::vector<DeviceParamsVariant>);
%ignore eduart::ring::SensorRingFactory::setDefaultDeviceParams;
%ignore eduart::ring::SensorRingFactory::buildDefaultParamsMap;

%include "sensorring/SensorRingFactory.hpp"

// Typed alternatives for std::variant-based methods
%extend eduart::ring::SensorRingFactory {
    void setDefaultVL53L8CXParams(eduart::device::VL53L8CX_Params params) {
        $self->setDefaultDeviceParams(std::move(params));
    }
    void setDefaultHTPA32Params(eduart::device::HTPA32_Params params) {
        $self->setDefaultDeviceParams(std::move(params));
    }
    void setDefaultWS2812bParams(eduart::device::WS2812b_Params params) {
        $self->setDefaultDeviceParams(std::move(params));
    }

    // expectBoard with explicit device params (replaces the std::variant overload).
    // Called from Python via the expectBoard() wrapper below.
    void _expectBoardWithDevices(
        eduart::device::SensorBoardParams board_params,
        eduart::device::VL53L8CX_Params* vl53,
        eduart::device::HTPA32_Params* htpa,
        eduart::device::WS2812b_Params* ws)
    {
        std::vector<eduart::ring::SensorRingFactory::DeviceParamsVariant> device_params;
        if (vl53) device_params.push_back(*vl53);
        if (htpa) device_params.push_back(*htpa);
        if (ws)   device_params.push_back(*ws);
        $self->expectBoard(std::move(board_params), std::move(device_params));
    }
}

// Factory returning raw pointer from unique_ptr (ownership transferred to Python)
%inline %{
namespace eduart { namespace ring {

  eduart::ring::SensorRing* SensorRingFactory_build(eduart::ring::SensorRingFactory* factory, eduart::ring::ValidationMode mode = eduart::ring::ValidationMode::Strict) {
    auto ptr = factory->build(mode);
    return ptr.release();
  }

  std::string SensorRingFactory_enumerate_str(eduart::ring::SensorRingFactory* factory) {
    factory->enumerate();
    return factory->printTopology();
  }

}}
%}
%newobject eduart::ring::SensorRingFactory_build;

%exception eduart::ring::SensorRingFactory_build {
    try {
        $action
    } catch (const std::exception& e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    }
}

// Attach factory wrappers as methods on the Python SensorRingFactory class
%pythoncode %{
def _SensorRingFactory_build(self, mode=ValidationMode_Strict):
    return SensorRingFactory_build(self, mode)
def _SensorRingFactory_enumerate(self):
    return SensorRingFactory_enumerate_str(self)

# Override expectBoard to accept optional device params:
#   factory.expectBoard(board_params)                           -> auto-discovery
#   factory.expectBoard(board_params, VL53L8CX_Params())       -> explicit devices
#   factory.expectBoard(board_params, VL53L8CX_Params(), WS2812b_Params())
_orig_expectBoard = SensorRingFactory.expectBoard
def _SensorRingFactory_expectBoard(self, board_params, *device_params):
    if not device_params:
        _orig_expectBoard(self, board_params)
    else:
        vl53 = htpa = ws = None
        for p in device_params:
            if isinstance(p, VL53L8CX_Params):
                vl53 = p
            elif isinstance(p, HTPA32_Params):
                htpa = p
            elif isinstance(p, WS2812b_Params):
                ws = p
            else:
                raise TypeError(f"Unknown device param type: {type(p).__name__}")
        self._expectBoardWithDevices(board_params, vl53, htpa, ws)

SensorRingFactory.build = _SensorRingFactory_build
SensorRingFactory.enumerate = _SensorRingFactory_enumerate
SensorRingFactory.expectBoard = _SensorRingFactory_expectBoard
%}

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
%ignore MeasurementManager::subscribeToStateChanges;
%ignore MeasurementManager::subscribeToDeviceGroup;
%ignore MeasurementManager::unsubscribe;
%ignore MeasurementManager::enqueueExtraAction;
%include "sensorring/manager/MeasurementManager.hpp"

// enqueueExtraAction helper: accepts a Python callable and wraps it in std::function<void()>
%{
static void Manager_enqueueExtraAction_py(
    eduart::manager::MeasurementManager* mgr, PyObject* callable) {
  Py_INCREF(callable);
  auto prevent_leak = std::shared_ptr<PyObject>(callable, [](PyObject* p) {
    PyGILState_STATE gstate = PyGILState_Ensure();
    Py_DECREF(p);
    PyGILState_Release(gstate);
  });
  mgr->enqueueExtraAction(
    [prevent_leak]() {
      PyGILState_STATE gstate = PyGILState_Ensure();
      PyObject* result = PyObject_CallObject(prevent_leak.get(), nullptr);
      Py_XDECREF(result);
      if (PyErr_Occurred()) PyErr_Print();
      PyGILState_Release(gstate);
    }
  );
}
%}
void Manager_enqueueExtraAction_py(eduart::manager::MeasurementManager* mgr, PyObject* callable);

// Make MeasurementManager(params, sensor_ring) use our factory (same API as C++).
%pythoncode %{
def _MeasurementManager_init(self, params, sensor_ring):
    other = make_MeasurementManager(params, sensor_ring)
    self.this = other.this
    self.thisown = other.thisown
MeasurementManager.__init__ = _MeasurementManager_init
%}

// WS2812b_Device: only expose the static setLight/syncLight helpers, not the full device class.
%{
static bool WS2812b_setLight(int mode, int red, int green, int blue) {
  return eduart::device::WS2812b_Device::setLight(
    static_cast<eduart::light::LightMode>(mode),
    static_cast<std::uint8_t>(red),
    static_cast<std::uint8_t>(green),
    static_cast<std::uint8_t>(blue));
}
static bool WS2812b_syncLight() {
  return eduart::device::WS2812b_Device::syncLight();
}
%}
bool WS2812b_setLight(int mode, int red, int green, int blue);
bool WS2812b_syncLight();


// DeviceGroup typed query helpers: access device-specific data without exposing
// the templated getDevicesOfType<T>() or the full device classes.
%{
static std::size_t DeviceGroup_getVL53L8CXCount(const eduart::device::DeviceGroup& group) {
  return group.getDevicesOfType<eduart::device::VL53L8CX_Device>().size();
}
static eduart::measurement::TofMeasurement DeviceGroup_getVL53L8CXMeasurement(const eduart::device::DeviceGroup& group, int index) {
  auto devs = group.getDevicesOfType<eduart::device::VL53L8CX_Device>();
  if (index < 0 || index >= static_cast<int>(devs.size()))
    throw std::out_of_range("VL53L8CX device index out of range");
  return devs[index]->getLatestRawMeasurement().first;
}
static std::size_t DeviceGroup_getHTPA32Count(const eduart::device::DeviceGroup& group) {
  return group.getDevicesOfType<eduart::device::HTPA32_Device>().size();
}
static eduart::measurement::ThermalMeasurement DeviceGroup_getHTPA32Measurement(const eduart::device::DeviceGroup& group, int index) {
  auto devs = group.getDevicesOfType<eduart::device::HTPA32_Device>();
  if (index < 0 || index >= static_cast<int>(devs.size()))
    throw std::out_of_range("HTPA32 device index out of range");
  return devs[index]->getLatestMeasurement().first;
}
%}
%catches(std::out_of_range) DeviceGroup_getVL53L8CXMeasurement;
%catches(std::out_of_range) DeviceGroup_getHTPA32Measurement;
std::size_t DeviceGroup_getVL53L8CXCount(const eduart::device::DeviceGroup& group);
eduart::measurement::TofMeasurement DeviceGroup_getVL53L8CXMeasurement(const eduart::device::DeviceGroup& group, int index);
std::size_t DeviceGroup_getHTPA32Count(const eduart::device::DeviceGroup& group);
eduart::measurement::ThermalMeasurement DeviceGroup_getHTPA32Measurement(const eduart::device::DeviceGroup& group, int index);

// HTPA32 calibration helper: start calibration on all HTPA32 devices reachable from a MeasurementManager.
%{
static void HTPA32_startCalibration(eduart::manager::MeasurementManager* mgr, int window) {
  auto devs = eduart::device::DeviceGroup(mgr->getSensorRing()->getDevices());
  for (auto* htpa32 : devs.getDevicesOfType<eduart::device::HTPA32_Device>()) {
    htpa32->startCalibration(static_cast<std::size_t>(window));
  }
}
%}
void HTPA32_startCalibration(eduart::manager::MeasurementManager* mgr, int window);


%rename (LogVerbosityToString) toString(LogVerbosity);
%include "sensorring/logger/LoggerTypes.hpp"


%catches(std::runtime_error) eduart::logger::Logger::log(const LogVerbosity verbosity, const std::string& msg) const;
%ignore Logger::log(const LogVerbosity, const std::stringstream);
%ignore Logger::subscribe;
%ignore Logger::unsubscribe;
%include "sensorring/logger/Logger.hpp" 


/****
 * Function-based subscription helpers (Python callable -> std::function)
 *
 * These allow Python code to use manager.subscribeToStateChanges(callback) and
 * manager.subscribeToDeviceGroup(DeviceType, callback) with plain Python
 * callables, mirroring the C++ lambda-based API.
 */

%{
#include <memory>

static eduart::Subscription* Logger_subscribe_py(
    eduart::logger::Logger* logger, PyObject* callable) {
  Py_INCREF(callable);
  auto prevent_leak = std::shared_ptr<PyObject>(callable, [](PyObject* p) {
    PyGILState_STATE gstate = PyGILState_Ensure();
    Py_DECREF(p);
    PyGILState_Release(gstate);
  });
  auto sub = logger->subscribe(
    [prevent_leak](const eduart::logger::LogVerbosity verbosity, const std::string& msg) {
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
  return new eduart::Subscription(std::move(sub));
}

static eduart::Subscription* Manager_subscribeToStateChanges_py(
    eduart::manager::MeasurementManager* mgr, PyObject* callable) {
  Py_INCREF(callable);
  auto prevent_leak = std::shared_ptr<PyObject>(callable, [](PyObject* p) {
    PyGILState_STATE gstate = PyGILState_Ensure();
    Py_DECREF(p);
    PyGILState_Release(gstate);
  });
  auto sub = mgr->subscribeToStateChanges(
    [prevent_leak](const eduart::manager::ManagerState state) {
      PyGILState_STATE gstate = PyGILState_Ensure();
      PyObject* py_state = PyLong_FromLong(static_cast<int>(state));
      PyObject* result = PyObject_CallFunctionObjArgs(prevent_leak.get(), py_state, nullptr);
      Py_XDECREF(py_state);
      Py_XDECREF(result);
      if (PyErr_Occurred()) PyErr_Print();
      PyGILState_Release(gstate);
    }
  );
  return new eduart::Subscription(std::move(sub));
}

static eduart::Subscription* Manager_subscribeToDeviceGroup_py(
    eduart::manager::MeasurementManager* mgr,
    eduart::device::DeviceType key,
    PyObject* callable) {
  Py_INCREF(callable);
  auto prevent_leak = std::shared_ptr<PyObject>(callable, [](PyObject* p) {
    PyGILState_STATE gstate = PyGILState_Ensure();
    Py_DECREF(p);
    PyGILState_Release(gstate);
  });
  auto sub = mgr->subscribeToDeviceGroup(key,
    [prevent_leak](const eduart::device::DeviceGroup& group) {
      PyGILState_STATE gstate = PyGILState_Ensure();
      swig_type_info* group_ti = SWIG_TypeQuery("eduart::device::DeviceGroup *");
      PyObject* py_group = SWIG_NewPointerObj(
        const_cast<eduart::device::DeviceGroup*>(&group), group_ti, 0);
      PyObject* result = PyObject_CallFunctionObjArgs(prevent_leak.get(), py_group, nullptr);
      Py_XDECREF(py_group);
      Py_XDECREF(result);
      if (PyErr_Occurred()) PyErr_Print();
      PyGILState_Release(gstate);
    }
  );
  return new eduart::Subscription(std::move(sub));
}
%}

// Declare the helpers for SWIG to generate Python wrappers
%newobject Logger_subscribe_py;
%newobject Manager_subscribeToStateChanges_py;
%newobject Manager_subscribeToDeviceGroup_py;

eduart::Subscription* Logger_subscribe_py(eduart::logger::Logger* logger, PyObject* callable);
eduart::Subscription* Manager_subscribeToStateChanges_py(eduart::manager::MeasurementManager* mgr, PyObject* callable);
eduart::Subscription* Manager_subscribeToDeviceGroup_py(eduart::manager::MeasurementManager* mgr, eduart::device::DeviceType key, PyObject* callable);

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

def _MeasurementManager_subscribeToDeviceGroup(self, device_type, callback):
    """Subscribe to a device group: callback(device_group)."""
    return Manager_subscribeToDeviceGroup_py(self, device_type, callback)
MeasurementManager.subscribeToDeviceGroup = _MeasurementManager_subscribeToDeviceGroup

def _MeasurementManager_enqueueExtraAction(self, action):
    """Queue a callable to run once in the next extra-actions slot of the state machine."""
    Manager_enqueueExtraAction_py(self, action)
MeasurementManager.enqueueExtraAction = _MeasurementManager_enqueueExtraAction
%}


/****
 * Client interfaces (director-enabled so Python classes can inherit and override)
 */

%template (TofMeasurementVector) std::vector<eduart::measurement::TofMeasurement>;
%template (ThermalMeasurementVector) std::vector<eduart::measurement::ThermalMeasurement>;

%feature("director") eduart::logger::LoggerClient;
%include "sensorring/logger/LoggerClient.hpp"

%feature("director") eduart::manager::MeasurementClient;
%include "sensorring/manager/MeasurementClient.hpp"
