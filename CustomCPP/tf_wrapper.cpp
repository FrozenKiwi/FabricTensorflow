#include "tf_wrapper.h"


#define SUCCEEDED(hr) ((hr) >= 0)

void free_buffer( void* data, size_t /*length*/ ) {
	free( data );
}

TF_Buffer* read_file( const char* file ) {
	FILE *f = nullptr;
	if (!SUCCEEDED( fopen_s( &f, file, "rb" ) ))
		 return nullptr;
	if (f == nullptr)
		return nullptr;

	fseek( f, 0, SEEK_END );
	long fsize = ftell( f );
	fseek( f, 0, SEEK_SET );  //same as rewind(f);                                            

	void* data = malloc( fsize );
	fread( data, fsize, 1, f );
	fclose( f );

	TF_Buffer* buf = TF_NewBuffer();
	buf->data = data;
	buf->length = fsize;
	buf->data_deallocator = free_buffer;
	return buf;
}


TFGraphWrapper* TFLoadGraph(const char* filepath, const char* inputOpName, int numInputs, int numOutputs)
{
	TF_Buffer* graph_def = read_file( filepath );
	if (graph_def == nullptr)
		return nullptr;

	auto wrapper = new TFGraphWrapper();
	wrapper->graph = TF_NewGraph();

	// Import graph_def into graph
	wrapper->status = TF_NewStatus();
	TF_ImportGraphDefOptions* graph_opts = TF_NewImportGraphDefOptions();
	TF_GraphImportGraphDef( wrapper->graph, graph_def, graph_opts, wrapper->status );
	if (TF_GetCode( wrapper->status ) != TF_OK) {
		fprintf( stderr, "ERROR: Unable to import graph %s", TF_Message( wrapper->status ) );
		return nullptr;
	}
	else {
		fprintf( stdout, "Successfully imported graph\n" );
	}

	// Create variables to store the size of the input and output variables
	// For now we only support float input/output
	int num_bytes_in = numInputs * sizeof( float );

	// Set input dimensions - this should match the dimensionality of the input in
	// the loaded graph.  We only support feeding one evaluation at a time,
	// so the first dimension is always 1.
	int64_t in_dims[] = { 1, numInputs };

	// ######################
	// Set up graph inputs
	// ######################

	//int n_ops = 700;
	//for (int i = 0; i < n_ops; i++)
	//{
	//	size_t pos = i;
	//	auto op_name = TF_OperationName( TF_GraphNextOperation( graph, &pos ) );
	//	std::cout << "Input: " << op_name << "\n";
	//}

	// Pass the graph and a string name of your input operation
	// (make sure the operation name is correct)
	// It _should_ be easier than this?  Anything more reliable?
	TF_Operation* input_op = TF_GraphOperationByName( wrapper->graph, inputOpName );
	if (input_op == nullptr)
	{
		return nullptr; // TODO... better error reporting.
	}

	// I kinda don't know what the hell this output is doing here...
	TF_Output input_opout = { input_op, 0 };
	wrapper->inputs.push_back( input_opout );

	// Create the input tensor using the dimension (in_dims) and size (num_bytes_in)
	// variables created earlier
	TF_Tensor* input = TF_AllocateTensor( TF_FLOAT, in_dims, 2, num_bytes_in );
	wrapper->input_values.push_back( input );

	// Optionally, you can check that your input_op and input tensors are correct
	// by using some of the functions provided by the C API.
	//CStr str;
	//str.printf( "Input op info: %d", TF_OperationNumOutputs( input_op ) );
	//DebugPrint( str.ToMSTR() );
	//str.printf( "Input data info: %d", TF_Dim( input, 0 ) );
	//DebugPrint( str.ToMSTR() );


	// ######################
	// Set up graph outputs (similar to setting up graph inputs)
	// ######################

	// Create vector to store graph output operations
	// So far it's always been a constant op_name
	TF_Operation* output_op = TF_GraphOperationByName( wrapper->graph, "output_node0" );
	TF_Output output_opout = { output_op, 0 };
	wrapper->outputs.push_back( output_opout );
	
	// Remember the number of outputs we are calculating
	//int output_dim = TF_OperationNumOutputs( output_op );
	//int input_dim = TF_OperationNumInputs( output_op );
	//pblock->SetValue( pb_output_dim, 0, output_dim );
	int64_t out_dims[] = { 1, numOutputs };
	int num_bytes_out = numOutputs * sizeof( float );

	// Similar to creating the input tensor, however here we don't yet have the
	// output values, so we use TF_AllocateTensor()
	TF_Tensor* output_value = TF_AllocateTensor( TF_FLOAT, out_dims, 2, num_bytes_out );
	wrapper->output_values.push_back( output_value );

	//// As with inputs, check the values for the output operation and output tensor
	//str.printf( "Output: %s", TF_OperationName( output_op ) );
	//DebugPrint( str.ToMSTR() );

	//str.printf( "Output info: %d", TF_Dim( output_value, 0 ) );
	//DebugPrint( str.ToMSTR() );

	TF_SessionOptions* sess_opts = TF_NewSessionOptions();
	wrapper->session = TF_NewSession( wrapper->graph, sess_opts, wrapper->status );
	if ( TF_GetCode( wrapper->status ) != TF_OK )
		return nullptr;
	
	return wrapper;
}
bool TFEvalGraph(TFGraphWrapper* wrapper, float* inputs, int numinputs, float* outputs, int numoutputs)
{
	// Write data to input tensor
	TF_Tensor* input_tensor = wrapper->input_values[0];
	// Ensure we don't try to write too many variables
	int tensor_dim = TF_Dim(input_tensor, 1);
	tensor_dim = tensor_dim < numinputs ? tensor_dim : numinputs;

	float* in_vals = static_cast<float*>(TF_TensorData(input_tensor));
	memcpy( in_vals, inputs, sizeof(float) * tensor_dim);

	// Call TF_SessionRun
	TF_SessionRun( wrapper->session, nullptr,
				   &wrapper->inputs[0], &wrapper->input_values[0], (int)wrapper->inputs.size(),
				   &wrapper->outputs[0], &wrapper->output_values[0], (int)wrapper->outputs.size(),
				   nullptr, 0, nullptr, wrapper->status );

	if (TF_GetCode( wrapper->status ) != TF_OK) {
		//CStr str;
		//str.printf( "ERROR: Unable to import graph %s", TF_Message( status ) );
		//DebugPrint( str.ToMSTR() );
		return false;
	}

	// Assign the values from the output tensor to a variable and iterate over them
	float* out_vals = static_cast<float*>(TF_TensorData( wrapper->output_values[0] ));
	memcpy(outputs, out_vals, sizeof(float) * numoutputs);
	return true;
}