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
	// configure the network
	// set up a COBA two-layer network with gaussian connectivity
	std::vector<int> gins;
	for (int i = 0; i < 3; i++)
	{
		std::string in_name = "input_" + std::to_string(i);
		gins.push_back(sim.createSpikeGeneratorGroup(in_name, 1, EXCITATORY_NEURON));
		// gins.push_back(sim.createGroup(in_name, 1, EXCITATORY_NEURON));
	}
	int gout=sim.createGroup("output", 1, EXCITATORY_NEURON);
	
	// Specify the Izhikevich parameters, in this case for class 1 excitability 
	// (regular spiking) neuron.
	// TODO, ask whether these numbers matter.
	// FIXME. I will keep these numbers for now.
	sim.setNeuronParameters(gout, 0.02f, 0.2f, -65.0f, 8.0f);
	std::vector<float> weights = {0.1f, 0.1f, 0.0f};
	float ALPHA_LTP = 0.10f;
        float ALPHA_LTD = -0.14f;
        float TAU_LTP = 20.0f;
        float TAU_LTD = 20.0f;
	for (int i = 0; i < 3; i++)
	{
		sim.connect(gins[i], gout, 
			"full", // Connects all neurons in the pre-synaptic group to all neurons in the post-synaptic group.
			RangeWeight(weights[i]), 
			1.0f, // 100% connection prob
			RangeDelay(1),
			RadiusRF(-1), 
			SYN_PLASTIC);
	}
	// sim.setESTDP(gout, true, STANDARD, ExpCurve(ALPHA_LTP/100, TAU_LTP, ALPHA_LTD/100, TAU_LTP));
	sim.setConductances(true);

	// ---------------- Transformed Network ---------------	
	
	
	// Config spiketimes
	std::vector<std::vector<int>> spikeTimes = {{5},
						{10},
						{15}};
	/*
	// Get more spikes
	for (int i = 0; i < 3; i++)	
	{
		for (int j = 1; j < 100; j++)
		{
			spikeTimes[i].push_back(100 * j + 5 * i);
		}
	}
	*/

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
	//
	std::vector<SpikeMonitor*> spkMons;

	SpikeMonitor* outMon = sim.setSpikeMonitor(gout,"DEFAULT");
	for (int i = 0; i < 3; i++)
	{
		spkMons.push_back(sim.setSpikeMonitor(gins[i],"DEFAULT"));
		sim.setConnectionMonitor(gins[i],gout,"DEFAULT");
	}
	
	// ---------------- RUN STATE -------------------
	watch.lap("runNetwork");
	for (int i = 0; i < 3; i++) { spkMons[i]->startRecording(); }
	outMon->startRecording();

	// run for a total of 10 seconds
	// at the end of each runNetwork call, SpikeMonitor stats will be printed
	for (int i=0; i<1; i++) {
		sim.runNetwork(1,0);
	}
	for (int i = 0; i < 3; i++) { spkMons[i]->stopRecording(); }
	outMon->stopRecording();

	for (int i = 0; i < 3; i++) { spkMons[i]->print(); }
	outMon->print();

	// print stopwatch summary
	watch.stop();
	
	return 0;
}
