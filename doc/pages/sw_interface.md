# Software interface of the library

The software interface of the Sensorring library is defined in four header files which are installed with the library.

## MeasurementManager.hpp
The header [MeasurementManager.hpp](../include/sensorring/MeasurementManager.hpp) defines the central class `manager::MeasurementManager`. The MeasurementManager runs a looping state machine in a thread which handles all communication with the sensors, times all measurements and processes them. The class must be instantiated with a parameter set of type `manager::ManagerParams`. The implementation of this parameter set is described below in the section *Parameters.hpp*.

```
    /**
     * Constructor
     * @param[in] params Parameter structure of the MeasurementManager
     */
    MeasurementManager(ManagerParams params);
```

```
    /**
     * Constructor
     * @param[in] params Parameter structure of the MeasurementManager
     * @param[in] observer Observer that is automatically registered before any other internal action. This ensures that no callbacks are missed.
     */
    MeasurementManager(ManagerParams params, MeasurementClient* observer);
```

```
    /**
     * Destructor
     */
    ~MeasurementManager();
```

``` 
    /**
     * Run one processing cycle of the state machine worker
     * @return error code
     */
    bool measureSome();
```

```
    /**
     * Start running the state machine worker loop in a thread
     * @return error code
     */
    bool startMeasuring();
```

```
    /**
     * Stop the state machine worker loop and close the thread
     * @return error code
     */
    bool stopMeasuring();
```

```
    /**
     * Register an observer with the MeasurementManager object
     * @param[in] observer Observer that is registered and gets notified on future events
     */
    void registerObserver(MeasurementClient* observer);
```

``` 
    /**
     * Get a string representation of the topology of the connected sensors
     * @return Formatted string describing the topology
     */
    std::string printTopology() const;
```

```
    /**
     * Get the health status of the state machine
     * @return Current worker state
     */
    WorkerState getWorkerState() const;
```

```
    /**
     * Get the parameters with which the MeasurementManager was initialized
     * @return Initial parameter struct
     */
    ManagerParams getParams() const;
```

```
    /**
     * Enable or disable the Time-of-Flight sensor measurements
     * @param[in] state enable signal
     */
    void enableTofMeasurement(bool state);
```

```
    /**
     * Enable or disable the thermal sensor measurements
     * @param[in] state enable signal
     */
    void enableThermalMeasurement(bool state);
```

```
    /**
     * Start a thermal calibration
     * @param[in] window number of thermal frames used for averaging
     * @return error code
     */
    bool startThermalCalibration(std::size_t window);
```

```
    /**
     * Stop any ongoing thermal calibration
     * @return error code
     */
    bool stopThermalCalibration();
```


## MeasurementClient.hpp
The header [MeasurementClient.hpp](../include/sensorring/MeasurementClient.hpp) defines the class `manager::MeasurementClient`. This class defines virtual callback methods to handle new measurements. To receive these new values a subclass of the MeasurementClient must implement the callback methods.

```
    /**
     * Callback method for state changes of the state machine worker
     * @param[in] status the new status of the state machine worker
     */
    virtual void onStateChange([[maybe_unused]] const WorkerState status) {};
```

```
    /**
     * Callback method for the log output of the sensorring library
     * @param[in] verbosity verbosity level of the log message
     * @param[in] msg       log message string
     */
    virtual void onOutputLog([[maybe_unused]] const LogVerbosity verbosity, [[maybe_unused]] const std::string msg) {};
```

```
  /**
   * Callback method for new Time-of-Flight sensor measurements. Returns a
   * vector of the raw measurements per sensor.
   * @param[in] measurement_vec the most recent Time-of-Flight sensor
   * measurements in the individual sensor coordinate frames
   */
  virtual void onRawTofMeasurement([[maybe_unused]] std::vector<measurement::TofMeasurement> measurement_vec) {};
```

```
  /**
   * Callback method for new Time-of-Flight sensor measurements.  Returns a
   * vector of the transformed measurements per sensor.
   * @param[in] measurement_vec the most recent Time-of-Flight sensor
   * measurements in the common transformed coordinate frame
   */
  virtual void onTransformedTofMeasurement([[maybe_unused]] std::vector<measurement::TofMeasurement> measurement_vec) {};
```

```
    /**
     * Callback method for new thermal sensor measurements. Returns one individual measurements of each thermal sensor.
     * @param[in] idx the index of the thermal sensor that that recorded the measurement. Index starts at zero (0) for the first thermal sensor and only counts active thermal sensors.
     * @param[in] measurement the most recent thermal sensor measurement of the specified sensor
     */
    virtual void onThermalMeasurement([[maybe_unused]] const std::size_t idx, [[maybe_unused]] const measurement::ThermalMeasurement measurement) {};
```

## LoggerClient.hpp
The header [LoggerClient.hpp](../include/sensorring/logger/LoggerClient.hpp) defines the class `manager::LoggerClient`. This class defines a virtual callback method to handle log messages. To receive these messages a subclass of the LoggerClient must implement the callback method.

```
    /**
     * Callback method for state changes of the state machine worker
     * @param[in] status the new status of the state machine worker
     */
    virtual void onStateChange([[maybe_unused]] const WorkerState status) {};
```

```
    /**
     * Callback method for the log output of the sensorring library
     * @param[in] verbosity verbosity level of the log message
     * @param[in] msg       log message string
     */
    virtual void onOutputLog([[maybe_unused]] const LogVerbosity verbosity, [[maybe_unused]] const std::string msg) {};
```

```
    /**
     * Callback method for new Time-of-Flight sensor measurements. Returns one combined measurement from all sensors.
     * @param[in] measurement the most recent combined Time-of-Flight sensor measurements
     */
    virtual void onTofMeasurement([[maybe_unused]] const measurement::TofMeasurement measurement) {};
```

```
    /**
     * Callback method for new thermal sensor measurements. Returns one individual measurements of each thermal sensor.
     * @param[in] idx the index of the thermal sensor that that recorded the measurement. Index starts at zero (0) for the first thermal sensor and only counts active thermal sensors.
     * @param[in] measurement the most recent thermal sensor measurement of the specified sensor
     */
    virtual void onThermalMeasurement([[maybe_unused]] const std::size_t idx, [[maybe_unused]] const measurement::ThermalMeasurement measurement) {};
```

## Parameters.hpp
The header [Parameters.hpp](../include/sensorring/types/Parameters.hpp) defines the parameter structure which is used to initialize the MeasurementManager. It describes the hardware topology as well as meta parameters of the system. The function of the individual parameters is explained [here](parameters.md). Refer to the header file for more implementation details.

```
    /**
    * @enum ThermalSensorParams
    * @brief Parameter structure of the thermal sensor of a sensor board. Not all sensor boards have thermal sensors.
    */
    struct sensor::ThermalSensorParams
```

```
    /**
    * @enum TofSensorParams
    * @brief Parameter structure of the Time-of-Flight sensor of a sensor board.
    */
    struct sensor::TofSensorParams
```

```
    /**
    * @enum SensorBoardParams
    * @brief Parameter structure of a sensor board. A sensor board is one circuit board.
    */
    struct sensor::SensorBoardParams
```

```
    /**
    * @enum BusParams
    * @brief Parameter structure of a communication bus. A bus is one communication interface e.g. can bus and has an arbitrary number of sensor boards connected.
    */
    struct bus::BusParams
```

```
    struct bus::BusParams
    /**
    * @enum RingParams
    * @brief Parameter structure of a sensor ring. The sensor ring is the abstraction of the whole sensor system and consists of an arbitrary number of communication interfaces.
    */
    struct ring::RingParams
```

```
    /**
    * @enum ManagerParams
    * @brief Meta parameter structure of the MeasurementManager. The MeasurementManager handles the timing and communication of the whole system by running a looping state machine. One measurement manager manages exactly one sensor ring.
    */
    struct manager::ManagerParams
```


## CustomTypes.hpp
The header [CustomTypes.hpp](../include/sensorring/types/install_types/CustomTypes.hpp) defines auxiliary types which are used in the interface of the library. Refer to the header file for implementation details.

```
    /**
    * @enum Vector3
    * @brief Lightweight implementation of a three dimensional vector of type double.
    */
    struct math::Vector3
    
    using PointCloud = std::vector<math::Vector3>;
```

```
    /**
    * @enum GenericGrayscaleImage
    * @brief Generic structure for images with one (1) channel. Typically used for grayscale images.
    */
    template<typename T, std::size_t RESOLUTION>
    struct measurement::GenericGrayscaleImage;

    using GrayscaleImage = GenericGrayscaleImage<std::uint8_t, THERMAL_RESOLUTION>;
    using TemperatureImage = GenericGrayscaleImage<double, THERMAL_RESOLUTION>;
```

```
    /**
    * @enum GenericRGBImage
    * @brief Generic structure for images with three (3) channels. Typically used for rgb images.
    */
    template<typename T, std::size_t RESOLUTION>
    struct measurement::GenericRGBImage

    using FalseColorImage = GenericRGBImage<std::uint8_t, THERMAL_RESOLUTION>;
```

```
    /**
    * @enum TofMeasurement
    * @brief Structure to hold the measurement from a single Time-of-Flight sensor or a combined measurment from multiple sensors.
    */
    struct measurement::TofMeasurement
```

```
    /**
    * @enum ThermalMeasurement
    * @brief Structure to hold the measurement from a single thermal sensor or.
    */
    struct measurement::ThermalMeasurement
```