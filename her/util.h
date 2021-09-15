#ifndef HER_UTIL_H_
#define HER_UTIL_H_
#include <sys/time.h>

#include <boost/mpi.hpp>

namespace her {

inline double GetCurrentTime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

uint32_t local_process_num(boost::mpi::communicator& comm) {
  uint32_t local_worker_num = 0;
  std::string processor_name = boost::mpi::environment::processor_name();
  std::vector<std::string> processor_names;

  boost::mpi::all_gather(comm, processor_name, processor_names);

  for (int i = 0; i < comm.size(); ++i) {
    if (processor_names[i] == processor_names[comm.rank()]) {
      local_worker_num++;
    }
  }
  return local_worker_num;
}

}  // namespace her
#endif  // HER_UTIL_H_
