## amazon-kinesis-video-streams-rtcp

The goal of Real-Time Transport Control Protocol (RTCP) is to provide
RTCP serialization and Deserialization functionalities. Along the library
also offers the ability to create and parse various RTCP packet types such
as Sender Reports (SR), Receiver Reports (RR), Full Intra Request (FIR), Picture
Loss Indication (PLI), Slice Loss Indication (SLI), Negative Acknowledgement
(NACK) packet, Receiver Estimated Maximum Bitrate (REMB) and Transport-wide
Congestion Control (TWCC) packets.

## What is RTCP

Real-Time Transport Control Protocol (RTCP) as defined in RFC 3550(https://datatracker.ietf.org/doc/html/rfc3550), is a companion
protocol used to monitor transmission statistics, quality of service and to provide
control information for RTP streams.

An RTCP packet consists of common header followed by structured data specific to RTCP
packet type.

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
    2. Initialize an RtcpPacket_t along with report to be send out.
       (e.g., Sender report or Receiver report).
    3. Call Rtcp_SerializeSenderReport() or Rtcp_SerializeReceiverReport() to serialize
       the RtcpPacket_t passed.

### Deserializer
    1. Call `Rtcp_Init()` to initialize the RTCP Context.
    2. Pass the serialized packet along with its length to `Rtcp_DeSerializePacket()`
       to deserialize the packet.

### Parse feedback packets received
    After deserializing the RTCP packet, RtcpPacket_t can be processed further to
    determine packet type and parse them using the following function :

    1. `Rtcp_ParseSenderReport()` to parse the sender report (SR) data.
    2. `Rtcp_ParseReceiverReport()` to parse the receiver report (RR)  data.
    3. `Rtcp_ParseFirPacket() to parse the Full Intra Request (FIR) packet.
    4. `Rtcp_ParsePliPacket() to parse the Picture Loss Indication (PLI) packet.
    5. `Rtcp_ParseSliPacket() to parse the Slice Loss Indication (SLI) packet.
    6. `Rtcp_ParseRembPacket() to parse the Receiver Estimated Maximum Bitrate (REMB) packet.
    7. `Rtcp_ParseNackPacket() to parse the Negative Acknowledgement (NACK) packet.
    8. `Rtcp_ParseTwccPacket() to parse the Transport Wide Congestion Control (TWCC) packet.

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

### Steps to build Unit Tests
1. Go to the root directory of this repository. (Make sure that the CMock
   submodule is cloned as described in [Checkout CMock Submodule](#checkout-cmock-submodule)).
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

### Steps to generate code coverage report of Unit Test
1. Run Unit Tests in [Steps to build Unit Tests](#steps-to-build-unit-tests).
2. Generate coverage.info in build folder:

    ```
    make coverage
    ```
3. Get code coverage by lcov:

    ```
    lcov --rc lcov_branch_coverage=1 -r coverage.info -o coverage.info '*test*' '*CMakeCCompilerId*' '*mocks*'
    ```
4. Generage HTML report in folder `CodecovHTMLReport`:

    ```
    genhtml --rc lcov_branch_coverage=1 --ignore-errors source coverage.info --legend --output-directory=CodecovHTMLReport
    ```

### Script to run Unit Test and generate code coverage report

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