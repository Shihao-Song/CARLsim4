/* * Copyright (c) 2016 Regents of the University of California. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* 3. The names of its contributors may not be used to endorse or promote
*    products derived from this software without specific prior written
*    permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* *********************************************************************************************** *
* CARLsim
* created by: (MDR) Micah Richert, (JN) Jayram M. Nageswaran
* maintained by:
* (MA) Mike Avery <averym@uci.edu>
* (MB) Michael Beyeler <mbeyeler@uci.edu>,
* (KDC) Kristofor Carlson <kdcarlso@uci.edu>
* (TSC) Ting-Shuo Chou <tingshuc@uci.edu>
* (HK) Hirak J Kashyap <kashyaph@uci.edu>
*
* CARLsim v1.0: JM, MDR
* CARLsim v2.0/v2.1/v2.2: JM, MDR, MA, MB, KDC
* CARLsim3: MB, KDC, TSC
* CARLsim4: TSC, HK
*
* CARLsim available from http://socsci.uci.edu/~jkrichma/CARLsim/
* Ver 12/31/2016
*/

// include CARLsim user interface
#include <carlsim.h>

// include stopwatch for timing
#include <stopwatch.h>

// spike generator
#include <spikegen_from_vector.h>

int main() {
	// keep track of execution time
	Stopwatch watch;
	

	// ---------------- CONFIG STATE -------------------
	// create a network on GPU
	int numGPUs = 1;
	int randSeed = 42;
	CARLsim sim("transform", CPU_MODE, USER, numGPUs, randSeed);

	// ---------------- Original Network ---------------	
	std::vector<int> gins;
	std::vector<int> gouts;

	for (int i = 0; i < 3; i++)
        {
                std::string in_name = "input_" + std::to_string(i);
                gins.push_back(sim.createSpikeGeneratorGroup(in_name, 1, EXCITATORY_NEURON));
        }
	for (int i = 0; i < 2; i++)
	{
		std::string out_name = "output_" + std::to_string(i);
		gouts.push_back(sim.createGroup(out_name, 1, EXCITATORY_NEURON));
	
		sim.setNeuronParameters(gouts[i], 0.02f, 0.2f, -65.0f, 8.0f);
	}

	// input[0] -> output[0]
	sim.connect(gins[0], gouts[0],
		"full",
		RangeWeight(0.1f),
		1.0f,
		RangeDelay(1),
		RadiusRF(-1),
		SYN_PLASTIC);

	// input[1] -> output[0]
	sim.connect(gins[1], gouts[0],
                "full",
                RangeWeight(0.1f),
                1.0f,
                RangeDelay(1),
                RadiusRF(-1),
                SYN_PLASTIC);

	// output[0] -> output[1]
	sim.connect(gouts[0], gouts[1],
                "full",
                RangeWeight(0.2f),
                1.0f,
                RangeDelay(1),
                RadiusRF(-1),
                SYN_PLASTIC);

	// input[2] -> output[1]
	sim.connect(gins[2], gouts[1],
                "full",
                RangeWeight(0.1f),
                1.0f,
                RangeDelay(1),
                RadiusRF(-1),
                SYN_PLASTIC);

        sim.setConductances(true);

	// Config spiketimes
	std::vector<std::vector<int>> spikeTimes = {{5},
						{10},
						{15}};

	std::vector<SpikeGeneratorFromVector> SGVs;
	for (int i = 0; i < 3; i++)
	{
		SGVs.emplace_back(spikeTimes[i]);
	}

	for (int i = 0; i < 3; i++)
	{
		sim.setSpikeGenerator(gins[i], &(SGVs[i]));
	}

	// ---------------- SETUP STATE -------------------
	watch.lap("setupNetwork");
	sim.setupNetwork();
	// set some monitors
	std::vector<SpikeMonitor*> spkMons;

	spkMons.push_back(sim.setSpikeMonitor(gins[0],"DEFAULT"));
	sim.setConnectionMonitor(gins[0],gouts[0],"DEFAULT");

	spkMons.push_back(sim.setSpikeMonitor(gins[1],"DEFAULT"));
	sim.setConnectionMonitor(gins[1],gouts[0],"DEFAULT");

	spkMons.push_back(sim.setSpikeMonitor(gins[2],"DEFAULT"));
	sim.setConnectionMonitor(gins[2],gouts[1],"DEFAULT");

	spkMons.push_back(sim.setSpikeMonitor(gouts[0],"DEFAULT"));
	sim.setConnectionMonitor(gouts[0],gouts[1],"DEFAULT");
	
	spkMons.push_back(sim.setSpikeMonitor(gouts[1],"DEFAULT"));

	// ---------------- SETUP STATE (Transform) -------------------

	// ---------------- RUN STATE -------------------
	watch.lap("runNetwork");
	for (int i = 0; i < spkMons.size(); i++) { spkMons[i]->startRecording(); }

	// run for a total of () seconds
	sim.setExternalCurrent(gouts[0], 2.0f);
	sim.runNetwork(0,10);
	sim.setExternalCurrent(gouts[0], 0.0f);
	
	sim.setExternalCurrent(gouts[1], 4.0f);
	sim.runNetwork(0,5);
	sim.setExternalCurrent(gouts[1], 0.0f);
	
	sim.runNetwork(1,0);

	for (int i = 0; i < spkMons.size(); i++) { spkMons[i]->stopRecording(); }

	for (int i = 0; i < spkMons.size(); i++) { spkMons[i]->print(); }

	// print stopwatch summary
	watch.stop();
	
	return 0;
}
