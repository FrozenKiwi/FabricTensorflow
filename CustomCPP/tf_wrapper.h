#pragma once

#include <c_api.h>
#include <vector>

class TFGraphWrapper
{
public:
	TF_Graph* graph;
	TF_Status* status;

	// Create vectors to store graph input operations and input tensors
	std::vector<TF_Output> inputs;
	std::vector<TF_Tensor*> input_values;

	// Create TF_Tensor* vector
	std::vector<TF_Output> outputs;
	std::vector<TF_Tensor*> output_values;

	TF_Session* session;
};

TFGraphWrapper* TFLoadGraph(const char* filepath, const char* inputOpName, int numInputs, int numOutputs);

bool TFEvalGraph(TFGraphWrapper* graph, float* inputs, int numinputs, float* outputs, int numoutputs);