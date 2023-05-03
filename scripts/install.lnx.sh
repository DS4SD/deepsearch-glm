sudo apt install -y libz-dev clang cmake subversion

echo "cleaning ... "

rm -rf ./externals
rm -rf ./build

echo "compiling ... "
mkdir build; cd build; /usr/local/bin/cmake .. -DPYTHON_EXECUTABLE=/usr/local/bin/python3; time make -j 8; cd ..

echo "linking ... "
cd ./python/nlp; ln -s ./build/andromeda_nlp.cpython-310-darwin.so ./andromeda_nlp.cpython-310-darwin.so; cd ../..

cd ./python/glm; ln -s ./build/andromeda_nlp.cpython-310-darwin.so ./andromeda_nlp.cpython-310-darwin.so; cd ../..
cd ./python/glm; ln -s ./build/andromeda_glm.cpython-310-darwin.so ./andromeda_glm.cpython-310-darwin.so; cd ../..

cd ./python/nlp_train; ln -s ../../build/andromeda_nlp.cpython-310-darwin.so ./andromeda_nlp.cpython-310-darwin.so; cd ../..

