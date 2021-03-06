/*
 * TaskScheduler.hpp
 *
 *  Created on: Dec 8, 2015
 *      Author: luca
 */

#include <iostream>
#include <ff/pipeline.hpp> // defines ff_pipeline and ff_Pipe

#include "InterTask.hpp"
#include "LineRecordReader.hpp"
#include "MapReduceWorker.hpp"
#include "MapReduceJob.hpp"
#include "MapResult.hpp"
#include "MapTask.hpp"
#include "TextInputFormat.hpp"

using namespace ff;

#ifndef TASKSCHEDULER_HPP_
#define TASKSCHEDULER_HPP_

template <typename MIK, typename MIV, typename MOK, typename MOV>
struct TaskScheduler: ff_node_t <Result<MIK,MIV,MOK,MOV>,Task<MIK,MIV,MOK,MOV>> {
	TaskScheduler (ff_loadbalancer *const lb , string file_name, int nWorkers,
			function<void(MIK key, MIV value, MapResult<MIK,MIV,MOK,MOV>*)> map_func)
		:lb(lb),file_name(file_name),nWorkers(nWorkers),map_func(map_func){
			input_format = new TextInputFormat(file_name, nWorkers);
			record_reader = new LineRecordReader(file_name);
		}
	void setRecordReader (RecordReader<MIK,MIV> *record_reader) {
		this->record_reader = record_reader;
	}
	void setInputFormat (InputFormat *inputFormat){
		this->input_format = input_format;
	}

	Task<MIK,MIV,MOK,MOV> *svc(Result<MIK,MIV,MOK,MOV>* result) {
		if(result==NULL){
			pair<off_t,off_t> *splits = input_format->getSplits();
			for(int i=0; i<nWorkers; i++)
				this->ff_send_out(new MapTask<MIK,MIV,MOK,MOV> (record_reader->clone(),splits[i],map_func));
			return (Task<MIK,MIV,MOK,MOV>*) GO_ON;
		}
		result->execute(this);
		if(completedMap==nWorkers)
			return (Task<MIK,MIV,MOK,MOV>*) EOF;
		return (Task<MIK,MIV,MOK,MOV>*) GO_ON;
	}

	bool completeMap(){
		return ++completedMap == nWorkers;
	}

private:
    unsigned int nWorkers, completedMap=0, completedReduce=0, onGoingInter=0;
    string file_name;
    ff_loadbalancer *lb;
    InputFormat *input_format;
    function<void(MIK key, MIV value, MapResult<MIK,MIV,MOK,MOV>*)> map_func;
    RecordReader<MIK,MIV> *record_reader;
};


#endif /* TASKSCHEDULER_HPP_ */
