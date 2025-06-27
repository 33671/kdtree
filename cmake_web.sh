emcmake cmake -S . -B build -DPLATFORM=Web -DCMAKE_POLICY_VERSION_MINIMUM=3.5
cd build && make && cd ..