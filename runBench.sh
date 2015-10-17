# make -f Makefile.old bench
DURATION=10

echo
echo "# small.json #######################"
echo -n "jason:     "
./bench jsonSample/small.json $DURATION 1 jason | grep "docs per second" 
echo -n "rapidjson: "
./bench jsonSample/small.json $DURATION 1 rapidjson | grep "docs per second"

echo
echo "# sample.json ######################"
echo -n "jason:     "
./bench jsonSample/sample.json $DURATION 1 jason | grep "docs per second"
echo -n "rapidjson: "
./bench jsonSample/sample.json $DURATION 1 rapidjson | grep "docs per second"

echo
echo "# sampleNoWhite.json ###############"
echo -n "jason:     "
./bench jsonSample/sampleNoWhite.json $DURATION 1 jason | grep "docs per second"
echo -n "rapidjson: "
./bench jsonSample/sampleNoWhite.json $DURATION 1 rapidjson | grep "docs per second"

echo
echo "# commits.json #####################"
echo -n "jason:     "
./bench jsonSample/commits.json $DURATION 1 jason | grep "docs per second"
echo -n "rapidjson: "
./bench jsonSample/commits.json $DURATION 1 rapidjson | grep "docs per second"

echo
