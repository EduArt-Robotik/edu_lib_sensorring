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
#include <stdexcept>
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
#include "sensorring/logger/LoggerClient.hpp"
#include "sensorring/platform/SensorringExport.hpp"
#include "sensorring/utils/CustomTypes.hpp"
#include "sensorring/utils/Math.hpp"
#include "sensorring/MeasurementClient.hpp"
#include "sensorring/MeasurementManager.hpp"
#include "sensorring/Parameters.hpp"
%}


// Include additional SWIG functionality
%include <stdint.i>
%include <exception.i>
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
%ignore eduart::math::Matrix3::operator[];
%extend eduart::math::Matrix3 {
    eduart::math::Vector3& __getitem__(int idx) {
        return $self->operator[](idx);
    }
    void __setitem__(int idx, const eduart::math::Vector3 &value) {
        $self->operator[](idx) = value;
    }
}
%template (VectorDataArray) std::array<double, 3>;
%template (MatrixDataArray) std::array<eduart::math::Vector3, 3>;
%include "sensorring/utils/Math.hpp"


%template (PointDataVector) std::vector<eduart::measurement::PointData>;
%include "sensorring/utils/CustomTypes.hpp"


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
%template (BoardParamVector) std::vector<eduart::sensor::SensorBoardParams>;
%include "sensorring/Parameters.hpp"


%feature("director") eduart::manager::MeasurementClient;
%rename (ManagerStateToString) eduart::manager::toString(ManagerState);
%template (TofMeasurementVector) std::vector<eduart::measurement::TofMeasurement>;
%template (ThermalMeasurementVector) std::vector<eduart::measurement::ThermalMeasurement>;
%include "sensorring/MeasurementClient.hpp"


%catches(std::runtime_error) eduart::manager::MeasurementManager::MeasurementManager(eduart::manager::ManagerParams params);
%catches(std::runtime_error) eduart::manager::MeasurementManager::measureSome(const LogVerbosity, const std::string);
%include "sensorring/MeasurementManager.hpp"


%feature("director") eduart::logger::LoggerClient;
%rename (LogVerbosityToString) toString(LogVerbosity);
%include "sensorring/logger/LoggerClient.hpp"


%catches(std::runtime_error) eduart::logger::Logger::log(const LogVerbosity, const std::string);
%ignore Logger::log(const LogVerbosity, const std::stringstream);
%include "sensorring/logger/Logger.hpp" 


%feature("director") eduart::wrapper::SensorringClient;
%inline %{
/* Python+Swig director have a problem with multiple inheritance. Only the first base class is directed correctly.
 * This creates a shim class that combines both client interfaces of the sensorring library into one class.
 */
namespace eduart {
namespace wrapper {

class SensorringClient : public eduart::manager::MeasurementClient,
                                  public eduart::logger::LoggerClient {
public:
    SensorringClient()
        : eduart::manager::MeasurementClient()
        , eduart::logger::LoggerClient() {}

    virtual ~SensorringClient() {}

    virtual void onStateChange([[maybe_unused]] const eduart::manager::ManagerState state) {};
    virtual void onRawTofMeasurement([[maybe_unused]] const std::vector<eduart::measurement::TofMeasurement>& measurement_vec) {};
    virtual void onTransformedTofMeasurement([[maybe_unused]] const std::vector<eduart::measurement::TofMeasurement>& measurement_vec) {};
    virtual void onThermalMeasurement([[maybe_unused]] const std::vector<eduart::measurement::ThermalMeasurement>& measurement_vec) {};
    virtual void onOutputLog([[maybe_unused]] eduart::logger::LogVerbosity verbosity, [[maybe_unused]] const std::string& msg) {};
};
} // namespace wrapper
} // namespace eduart

%}