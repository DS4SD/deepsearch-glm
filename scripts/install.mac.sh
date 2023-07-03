
echo "cleaning ... "
rm -rf ./externals
rm -rf ./build

echo "compiling ... "
#mkdir build; cd build; /usr/local/bin/cmake .. -DPYTHON_EXECUTABLE=/usr/local/bin/python3; make -j; cd ..
mkdir build; cd build; /usr/local/bin/cmake .. -DPYTHON_EXECUTABLE=/usr/local/bin/python3; make -j 1; cd ..


echo "linking ... "

cd deepsearch_glm; rm *.so; 
ln -s ../build/andromeda_nlp.cpython-39-darwin.so ./andromeda_nlp.cpython-39-darwin.so;
ln -s ../build/andromeda_glm.cpython-39-darwin.so ./andromeda_glm.cpython-39-darwin.so;
ln -s ../build/andromeda_nlp.cpython-310-darwin.so ./andromeda_nlp.cpython-310-darwin.so;
ln -s ../build/andromeda_glm.cpython-310-darwin.so ./andromeda_glm.cpython-310-darwin.so;
cd ..


cd ./python/nlp; rm *.so; cd ../..
cd ./python/nlp; ln -s ../../build/andromeda_nlp.cpython-39-darwin.so ./andromeda_nlp.cpython-39-darwin.so; cd ../..
cd ./python/nlp; ln -s ../../build/andromeda_glm.cpython-39-darwin.so ./andromeda_glm.cpython-39-darwin.so; cd ../..
cd ./python/nlp; ln -s ../../build/andromeda_nlp.cpython-310-darwin.so ./andromeda_nlp.cpython-310-darwin.so; cd ../..
cd ./python/nlp; ln -s ../../build/andromeda_glm.cpython-310-darwin.so ./andromeda_glm.cpython-310-darwin.so; cd ../..

cd ./python/glm; rm *.so; cd ../..
cd ./python/glm; ln -s ../../build/andromeda_nlp.cpython-39-darwin.so ./andromeda_nlp.cpython-39-darwin.so; cd ../..
cd ./python/glm; ln -s ../../build/andromeda_glm.cpython-39-darwin.so ./andromeda_glm.cpython-39-darwin.so; cd ../..
cd ./python/glm; ln -s ../../build/andromeda_nlp.cpython-310-darwin.so ./andromeda_nlp.cpython-310-darwin.so; cd ../..
cd ./python/glm; ln -s ../../build/andromeda_glm.cpython-310-darwin.so ./andromeda_glm.cpython-310-darwin.so; cd ../..

echo "done!"
