## Project Description

**AEB** is an advanced active safety system that assists drivers in avoiding or mitigating collisions with other vehicles or obstacles. It detects potential frontal collisions and automatically engages the vehicle's braking system to reduce speed or come to a complete stop, aiming to prevent or minimize impact.

To implement this system, **POSIX** (Portable Operating System Interface) concepts are employed. POSIX defines programming interfaces and operating system interfaces to maintain compatibility between different operating systems, facilitating portability and interoperability of applications.

## Project Objective

This project is part of the final course in Automotive Propulsion Engineering, aka **Residência Tecnológica Stellantis 2024, SWT2**. It is dedicated to the development of an Automotive Emergency Braking (AEB) system, aligning with Stellantis’ interests. The project takes a multidisciplinary approach, demonstrating the student's understanding of various topics covered during the course. Key topics include Vehicle Modeling and Simulation, Agile Methodologies, Vehicular Networks, Vehicle Function Allocation, Automotive Embedded Software Modeling and Testing, and the Implementation of Automotive Control Systems.

## Main Features

- **Collision Detection**: Continuous monitoring of the vehicle's frontal environment to identify potential imminent collisions.
- **Automatic Braking Activation**: In the event of a collision risk detection, the system automatically engages the brakes to reduce speed or stop the vehicle.
- **Driver Notifications**: Visual and audible alerts to inform the driver about potential hazards and actions taken by the system.

## Technologies Used

- **Programming Language**: C, leveraging POSIX-compatible libraries and system calls to ensure portability and efficiency.
- **POSIX-Compatible Operating Systems**: Development is focused on systems adhering to POSIX standards, ensuring greater code compatibility and portability.

## Project Structure

The project's directory structure is organized as follows:

- **`src/`**: Contains the main source code of the AEB system.
- **`test/`**: Holds unit tests for validating the system's modules.
- **`docs/`**: Dedicated to project documentation, including specifications and manuals.
- **`.github/`**: Utilized for GitHub workflows and automated actions.
- **`bin/`**: Stores binary files generated during the build process.
- **`cts/`**: Specific generated databases.
- **`inc/`**: Contains header files used in the source code.
- **`obj/`**: Holds object files created during compilation.
- **`Makefile`**: Script to automate the build process and execute tests.

This organization enhances code navigation and maintenance, ensuring a clear separation of responsibilities among different project components.

## Setup and Execution

To build and run the project, follow the steps below:

1. **Prerequisites**:
   - A POSIX-compliant operating system.
   - A C compiler (e.g., GCC) installed on your system.

2. **Building the Project**:
   - Navigate to the project's root directory.
   - Run `make` to compile the source code.

3. **Running the System**:
   - After a successful build, execute the system with `./aeb_system`.

4. **Running Tests**:
   - To execute unit tests, use `make test`.

## Contribution

Contributions are welcome! To contribute:

1. Fork this repository.
2. Create a new branch for your feature or fix: `git checkout -b my-feature`.
3. Make the desired changes and commit: `git commit -m 'My new feature'`.
4. Push to the remote repository: `git push origin my-feature`.
5. Open a Pull Request for review.

## License

This project is licensed under the MIT License. For more details, refer to the `LICENSE` file in the repository.

---