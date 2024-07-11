## amazon-kinesis-video-streams-rtcp

The goal of this library is to provide Real-Time Transport Control Protocol
(RTCP) serialization and deserialization functionalities. The library supports
creating and parsing the following RTCP packets:
* Sender Reports (SR).
* Receiver Reports (RR).
* Full Intra Request (FIR).
* Picture Loss Indication (PLI).
* Slice Loss Indication (SLI).
* Negative Acknowledgement (NACK) report.
* Receiver Estimated Maximum Bitrate (REMB) report.
* Transport-Wide Congestion Control (TWCC) report.

## What is RTCP

[Real-Time Transport Control Protocol (RTCP)](https://en.wikipedia.org/wiki/RTP_Control_Protocol),
as defined in [RFC 3550](https://datatracker.ietf.org/doc/html/rfc3550), is a
Control Protocol, to allow monitoring of transmission statistics, quality of
service and to provide control information for RTP streams.

An RTCP packet consists of a common header followed by structured data specific
to the RTCP packet type.

```
RTCP Header:

0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|V=2|P|    FMT     |       PT      |             length         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

```

## Using the library

### Serializer

1. Call `Rtcp_Init()` to initialize the RTCP Context.
2. Populate `RtcpSenderReport_t` or `RtcpReceiverReport_t` with the report data
   to be sent out.
3. Call `Rtcp_SerializeSenderReport()` or `Rtcp_SerializeReceiverReport()` to
   serialize the RTCP report.

### Deserializer

1. Call `Rtcp_Init()` to initialize the RTCP Context.
2. Pass the serialized packet, received over the wire, to the
   `Rtcp_DeSerializePacket()` to deserialize the packet.
3. Use one of the following APIs to further parse the deserialized packet
   according to the value of `RtcpPacket_t.header.packetType`:
    * Use `Rtcp_ParseSenderReport()` to parse the Sender Report (SR).
    * Use `Rtcp_ParseReceiverReport()` to parse the Receiver Report (RR).
    * Use `Rtcp_ParseFirPacket()` to parse the Full Intra Request (FIR).
    * Use `Rtcp_ParsePliPacket()` to parse the Picture Loss Indication (PLI).
    * Use `Rtcp_ParseSliPacket()` to parse the Slice Loss Indication (SLI).
    * Use `Rtcp_ParseRembPacket()` to parse the Receiver Estimated Maximum
      Bitrate (REMB) report.
    * Use `Rtcp_ParseNackPacket()` to parse the Negative Acknowledgement (NACK)
      report.
    * Use `Rtcp_ParseTwccPacket()` to parse the Transport-Wide Congestion
      Control (TWCC) report.

## Building Unit Tests

### Platform Prerequisites

- For running unit tests:
    - C99 compiler like gcc.
    - CMake 3.13.0 or later.
    - Ruby 2.0.0 or later (It is required for the CMock test framework that we
      use).
- For running the coverage target, gcov and lcov are required.

### Checkout CMock Submodule

By default, the submodules in this repository are configured with `update=none`
in [.gitmodules](./.gitmodules) to avoid increasing clone time and disk space
usage of other repositories.

To build unit tests, the submodule dependency of CMock is required. Use the
following command to clone the submodule:

```sh
git submodule update --checkout --init --recursive test/CMock
```

### Steps to Build and Run Unit Tests

1. Go to the root directory of this repository. Make sure that the CMock
   submodule is cloned as described in [Checkout CMock Submodule](#checkout-cmock-submodule).
2. Run the following command to generate Makefiles:

    ```sh
    cmake -S test/unit-test -B build/ -G "Unix Makefiles" \
     -DCMAKE_BUILD_TYPE=Debug \
     -DBUILD_CLONE_SUBMODULES=ON \
     -DCMAKE_C_FLAGS='--coverage -Wall -Wextra -Werror -DNDEBUG'
    ```
3. Run the following command to build the library and unit tests:

    ```sh
    make -C build all
    ```
4. Run the following command to execute all tests and view results:

    ```sh
    cd build && ctest -E system --output-on-failure
    ```

### Steps to Generate Code Coverage Report

1. Run Unit Tests in [Steps to Build and Run Unit Tests](#steps-to-build-and-run-unit-tests).
2. Generate coverage.info in the `build` folder:

    ```
    make coverage
    ```
3. Get code coverage by `lcov`:

    ```
    lcov --rc lcov_branch_coverage=1 -r coverage.info -o coverage.info '*test*' '*CMakeCCompilerId*' '*mocks*'
    ```
4. Generage HTML report in the folder `CodecovHTMLReport`:

    ```
    genhtml --rc lcov_branch_coverage=1 --ignore-errors source coverage.info --legend --output-directory=CodecovHTMLReport
    ```

### Script to Run Unit Test and Generate Code Coverage Report

```sh
git submodule update --init --recursive --checkout test/CMock
cmake -S test/unit-test -B build/ -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DBUILD_CLONE_SUBMODULES=ON -DCMAKE_C_FLAGS='--coverage -Wall -Wextra -Werror -DNDEBUG -DLIBRARY_LOG_LEVEL=LOG_DEBUG'
make -C build all
cd build
ctest -E system --output-on-failure
make coverage
lcov --rc lcov_branch_coverage=1 -r coverage.info -o coverage.info '*test*' '*CMakeCCompilerId*' '*mocks*'
genhtml --rc lcov_branch_coverage=1 --ignore-errors source coverage.info --legend --output-directory=CodecovHTMLReport
```

## Security

See [CONTRIBUTING](CONTRIBUTING.md#security-issue-notifications) for more information.

## License

This project is licensed under the Apache-2.0 License.
