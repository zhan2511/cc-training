#include <iostream>
#include <cstdlib>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

double cwnd = 1; // window size
unsigned int ssthresh = 10; // ssthresh
bool first_ack = true; // first ack received
uint64_t SampleRTT = 0; // sample RTT
uint64_t BaseRTT = 0; // base RTT
double ExpectedRate = 0; // expected rate
double ActualRate = 0; // actual rate
double Diff = 0; // difference between expected and actual rate
double alpha = 0.4;
double beta = 0.8;

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
         << " window size is " << cwnd << endl;
  }

  return cwnd;
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

  // // Timeout
  // if (after_timeout && sequence_number != 0 && sequence_number != 1)
  // {
  //   cout << "timeout : " << sequence_number << endl;
  //   ssthresh = (the_window_size / 2) < 2 ? 2 : (the_window_size / 2);
  //   the_window_size = 1;
  //   cout << "cwnd:" << the_window_size << " ssthresh:" << ssthresh << endl;
  // }
  // else
  // {}

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

  cout << "num_acked:" << sequence_number_acked << endl;

  if (first_ack)
  {
    first_ack = false;

    SampleRTT = timestamp_ack_received - send_timestamp_acked;
    BaseRTT = SampleRTT; // set base RTT
    ExpectedRate = (cwnd / SampleRTT); // expected rate
    cout << "BaseRTT:" << BaseRTT << endl;
    cout << "SampleRTT:" << SampleRTT << endl;
    cwnd = cwnd + 4;
    cout << "cwnd:" << cwnd << " ssthresh:" << ssthresh << endl;
  }else
  {
    SampleRTT = timestamp_ack_received - send_timestamp_acked;
    ExpectedRate = cwnd / BaseRTT; // expected rate
    ActualRate = cwnd / SampleRTT; // actual rate
    Diff = ExpectedRate - ActualRate;

    if (cwnd < ssthresh)
    {
      // slow start
      if (Diff < alpha)
      {
        cwnd = cwnd + 4;
      }else if (Diff > beta)
      {
        ssthresh = cwnd * 0.6 < 2 ? 2 : cwnd * 0.6;
        cwnd = ssthresh;
      }
      else
      {
        // do nothing
      }
      cout << "cwnd:" << cwnd << " ssthresh:" << ssthresh << endl;
    }
    else
    {
      // congestion avoidance
      SampleRTT = timestamp_ack_received - send_timestamp_acked;
      ExpectedRate = cwnd / BaseRTT; // expected rate
      ActualRate = cwnd / SampleRTT; // actual rate
      Diff = ExpectedRate - ActualRate;
      if (Diff < alpha)
      {
        cwnd = cwnd + 2;
      }else if (Diff > beta)
      {
        cwnd = cwnd - 2;
      }
      else
      {
        // do nothing
      }
      cout << "cwnd:" << cwnd << " ssthresh:" << ssthresh << endl;
    }

    if (BaseRTT > SampleRTT)
    {
      BaseRTT = SampleRTT; // update base RTT
    }
    cout << "BaseRTT:" << BaseRTT << endl;
    cout << "SampleRTT:" << SampleRTT << endl;
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
  return 1000; /* timeout of one second */
}
