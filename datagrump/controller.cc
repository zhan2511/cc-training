#include <iostream>
#include <cstdlib>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

/* Default constructor */
Controller::Controller(const bool debug)
    : debug_(debug)
{
}

/* Get current window size, in datagrams */
unsigned int Controller::window_size()
{
  /* Default: fixed window size of 100 outstanding datagrams */

  if (debug_)
  {
    cerr << "At time " << timestamp_ms()
         << " window size is " << the_window_size << endl;
  }

  return the_window_size;
}

/* A datagram was sent */
void Controller::datagram_was_sent(const uint64_t sequence_number,
                                   /* of the sent datagram */
                                   const uint64_t send_timestamp,
                                   /* in milliseconds */
                                   const bool after_timeout
                                   /* datagram was sent because of a timeout */)
{
  /* Default: take no action */

  // Timeout
  if (after_timeout)
  {
    cout << "timeout : " << sequence_number << endl;
    ssthresh = the_window_size / 2;
    the_window_size = 1;
  }
  else
  {
  }

  if (debug_)
  {
    cerr << "At time " << send_timestamp
         << " sent datagram " << sequence_number << " (timeout = " << after_timeout << ")\n";
  }
}

/* An ack was received */
void Controller::ack_received(const uint64_t sequence_number_acked,
                              /* what sequence number was acknowledged */
                              const uint64_t send_timestamp_acked,
                              /* when the acknowledged datagram was sent (sender's clock) */
                              const uint64_t recv_timestamp_acked,
                              /* when the acknowledged datagram was received (receiver's clock)*/
                              const uint64_t timestamp_ack_received)
/* when the ack was received (by sender) */
{
  /* Default: take no action */

  cout << "num_acked:" << sequence_number_acked
       << "    last_ack:" << last_ack << endl;

  // Repeated ACK
  if (last_ack == 0)
  {
    last_ack = sequence_number_acked;
    // initial RTO
    SampleRTT = timestamp_ack_received - send_timestamp_acked;
    EstimatedRTT = SampleRTT;
    DevRTT = SampleRTT / 2;
    RTO = EstimatedRTT + 4 * DevRTT;
    cout << "SampleRTT:" << SampleRTT
         << " EstimatedRTT:" << EstimatedRTT
         << " DevRTT:" << DevRTT
         << " RTO:" << RTO << endl;
  }
  else if (sequence_number_acked == last_ack && last_ack != 0)
  {
    cout << "got same ack" << sequence_number_acked << endl;
  }
  else
  {
    last_ack = sequence_number_acked;
    if (the_window_size < ssthresh)
    {
      // slow start : increase by 1 every ack
      the_window_size = the_window_size + 1;
    }
    else
    {
      // congestion avoidance : increase by 1 every round of acks
      the_window_size = the_window_size + 1 / the_window_size;
    }
    // calculate RTO
    SampleRTT = timestamp_ack_received - send_timestamp_acked;
    EstimatedRTT = 0.875 * EstimatedRTT + 0.125 * SampleRTT;
    DevRTT = 0.75 * DevRTT + 0.25 * abs(static_cast<int64_t>(SampleRTT - EstimatedRTT));
    RTO = EstimatedRTT + 4 * DevRTT;
    cout << "SampleRTT:" << SampleRTT
         << " EstimatedRTT:" << EstimatedRTT
         << " DevRTT:" << DevRTT
         << " RTO:" << RTO << endl;
  }

  if (debug_)
  {
    cerr << "At time " << timestamp_ack_received
         << " received ack for datagram " << sequence_number_acked
         << " (send @ time " << send_timestamp_acked
         << ", received @ time " << recv_timestamp_acked << " by receiver's clock)"
         << endl;
  }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms()
{
  return RTO; /* timeout of one second */
}
