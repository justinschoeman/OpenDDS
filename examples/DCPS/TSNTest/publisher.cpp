// -*- C++ -*-
// *******************************************************************
//
// (c) Copyright 2006, Object Computing, Inc.
// All Rights Reserved.
//
// *******************************************************************

#include "StockQuoterTypeSupportImpl.h"
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/PublisherImpl.h>
#include <ace/streams.h>
#include <ace/OS_NS_unistd.h>
#include <orbsvcs/Time_Utilities.h>

#include "dds/DCPS/StaticIncludes.h"

// constants for Stock Quoter domain Id, types, and topic
DDS::DomainId_t QUOTER_DOMAIN_ID = 1066;
const char* QUOTER_QUOTE_TYPE = "Quote Type";
const char* QUOTER_QUOTE_TOPIC = "Stock Quotes";
const char* QUOTER_EXCHANGE_EVENT_TYPE = "Exchange Event Type";
const char* QUOTER_EXCHANGE_EVENT_TOPIC = "Stock Exchange Events";

const char* STOCK_EXCHANGE_NAME = "Test Stock Exchange";

TimeBase::TimeT get_timestamp() {
  TimeBase::TimeT retval;
  ACE_hrtime_t t = ACE_OS::gethrtime ();
  ORBSVCS_Time::hrtime_to_TimeT (retval, t );
  return retval;
}

int ACE_TMAIN (int argc, ACE_TCHAR *argv[]) {

  DDS::DomainParticipantFactory_var dpf = DDS::DomainParticipantFactory::_nil();
  DDS::DomainParticipant_var participant = DDS::DomainParticipant::_nil();

  try {
    // Initialize, and create a DomainParticipant

    dpf = TheParticipantFactoryWithArgs(argc, argv);

    participant = dpf->create_participant(
      QUOTER_DOMAIN_ID,
      PARTICIPANT_QOS_DEFAULT,
      DDS::DomainParticipantListener::_nil(),
      ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);

    if (CORBA::is_nil (participant.in ())) {
      cerr << "create_participant failed." << endl;
      ACE_OS::exit(1);
    }

    // Create a publisher for the two topics
    // (PUBLISHER_QOS_DEFAULT is defined in Marked_Default_Qos.h)
    DDS::Publisher_var pub =
      participant->create_publisher(PUBLISHER_QOS_DEFAULT,
                                    DDS::PublisherListener::_nil(),
                                    ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
    if (CORBA::is_nil (pub.in ())) {
      cerr << "create_publisher failed." << endl;
      ACE_OS::exit(1);
    }

    // Register the Quote type
    StockQuoter::QuoteTypeSupport_var quote_servant
      = new StockQuoter::QuoteTypeSupportImpl();
    if (DDS::RETCODE_OK != quote_servant->register_type(participant.in (),
                                                        QUOTER_QUOTE_TYPE)) {
      cerr << "register_type for " << QUOTER_QUOTE_TYPE << " failed." << endl;
      ACE_OS::exit(1);
    }

    // Register the ExchangeEvent type
    StockQuoter::ExchangeEventTypeSupport_var exchange_evt_servant
      = new StockQuoter::ExchangeEventTypeSupportImpl();

    if (DDS::RETCODE_OK != exchange_evt_servant->register_type(participant.in (),
                                                               QUOTER_EXCHANGE_EVENT_TYPE)) {
      cerr << "register_type for " << QUOTER_EXCHANGE_EVENT_TYPE
           << " failed." << endl;
      ACE_OS::exit(1);
    }

    // Get QoS to use for our two topics
    // Could also use TOPIC_QOS_DEFAULT instead
    DDS::TopicQos default_topic_qos;
    participant->get_default_topic_qos(default_topic_qos);

    // Create a topic for the Quote type...
    DDS::Topic_var quote_topic =
      participant->create_topic (QUOTER_QUOTE_TOPIC,
                                 QUOTER_QUOTE_TYPE,
                                 default_topic_qos,
                                 DDS::TopicListener::_nil(),
                                 ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
    if (CORBA::is_nil (quote_topic.in ())) {
      cerr << "create_topic for " << QUOTER_QUOTE_TOPIC << " failed." << endl;
      ACE_OS::exit(1);
    }

    // .. and another topic for the Exchange Event type
    DDS::Topic_var exchange_evt_topic =
      participant->create_topic (QUOTER_EXCHANGE_EVENT_TOPIC,
                                 QUOTER_EXCHANGE_EVENT_TYPE,
                                 default_topic_qos,
                                 DDS::TopicListener::_nil(),
                                 ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
    if (CORBA::is_nil (exchange_evt_topic.in ())) {
      cerr << "create_topic for " << QUOTER_EXCHANGE_EVENT_TOPIC << " failed." << endl;
      ACE_OS::exit(1);
    }

    // Get the default QoS for our Data Writers
    // Could also use DATAWRITER_QOS_DEFAULT
    DDS::DataWriterQos dw_default_qos;
    pub->get_default_datawriter_qos (dw_default_qos);

    // Create a DataWriter for the Quote topic
    DDS::DataWriter_var quote_base_dw =
      pub->create_datawriter(quote_topic.in (),
                             dw_default_qos,
                             DDS::DataWriterListener::_nil(),
                             ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
    if (CORBA::is_nil (quote_base_dw.in ())) {
      cerr << "create_datawriter for " << QUOTER_QUOTE_TOPIC << " failed." << endl;
      ACE_OS::exit(1);
    }
    StockQuoter::QuoteDataWriter_var quote_dw
      = StockQuoter::QuoteDataWriter::_narrow(quote_base_dw.in());
    if (CORBA::is_nil (quote_dw.in ())) {
      cerr << "QuoteDataWriter could not be narrowed"<< endl;
      ACE_OS::exit(1);
    }

    // Create a DataWriter for the Exchange Event topic
    DDS::DataWriter_var exchange_evt_base_dw =
      pub->create_datawriter(exchange_evt_topic.in (),
                             dw_default_qos,
                             DDS::DataWriterListener::_nil(),
                             ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
    if (CORBA::is_nil (exchange_evt_base_dw.in ())) {
      cerr << "create_datawriter for " << QUOTER_EXCHANGE_EVENT_TOPIC << " failed." << endl;
      ACE_OS::exit(1);
    }
    StockQuoter::ExchangeEventDataWriter_var exchange_evt_dw
      = StockQuoter::ExchangeEventDataWriter::_narrow(exchange_evt_base_dw.in());
    if (CORBA::is_nil (exchange_evt_dw.in ())) {
      cerr << "ExchangeEventDataWriter could not be narrowed"<< endl;
      ACE_OS::exit(1);
    }

    // Register the Exchange Event and the two Quoted securities (SPY and MDY)
    StockQuoter::ExchangeEvent ex_evt;
    ex_evt.exchange = STOCK_EXCHANGE_NAME;
    DDS::InstanceHandle_t exchange_handle = exchange_evt_dw->register_instance(ex_evt);

    StockQuoter::Quote spy;
    spy.id = 0; // key?
    DDS::InstanceHandle_t spy_handle = quote_dw->register_instance(spy);

    // Publish...

    StockQuoter::ExchangeEvent opened;
    opened.exchange = STOCK_EXCHANGE_NAME;
    opened.event = StockQuoter::TRADING_OPENED;
    opened.timestamp = get_timestamp();

    cout << "Publishing TRADING_OPENED" << endl;
    DDS::ReturnCode_t ret = exchange_evt_dw->write(opened, exchange_handle);
    if (ret != DDS::RETCODE_OK) {
      ACE_ERROR ((LM_ERROR, ACE_TEXT("(%P|%t) ERROR: OPEN write returned %d.\n"), ret));
    }

// urk - ugly hardcoded schedule
#define NSEC_PER_SEC 1000000000L
#define EARLY_OFFSET_NS 100000L
#define INTERVAL_NS 500000L

    StockQuoter::Quote spy_quote;
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    unsigned long target_time = now.tv_sec * NSEC_PER_SEC; // FIXME - ASSUMES TSN WINDOW IS ALIGNED WITH WHOLE SECONDS!!!
    unsigned long sched_wake_time = target_time - EARLY_OFFSET_NS;

    ACE_Time_Value quarterSecond( 0, 250000 );
    for ( int i = 0; i < 100000; ++i ) {
      now.tv_sec = sched_wake_time / NSEC_PER_SEC;
      now.tv_nsec = sched_wake_time % NSEC_PER_SEC;
      clock_gettime(CLOCK_REALTIME, &now);
      unsigned long act_wake_time = now.tv_sec * NSEC_PER_SEC;
      spy_quote.target_time = target_time;
      spy_quote.sched_wake_time = sched_wake_time;
      spy_quote.act_wake_time = act_wake_time;
      ret = quote_dw->write(spy_quote, spy_handle);
      if (ret != DDS::RETCODE_OK) {
        ACE_ERROR ((LM_ERROR, ACE_TEXT("(%P|%t) ERROR: SPY write returned %d.\n"), ret));
      }
      cout << "Publishing SPY Quote: " << target_time << endl;
      target_time += INTERVAL_NS;
      sched_wake_time += INTERVAL_NS;
    }

    StockQuoter::ExchangeEvent closed;
    closed.exchange = STOCK_EXCHANGE_NAME;
    closed.event = StockQuoter::TRADING_CLOSED;
    closed.timestamp = get_timestamp();

    cout << "Publishing TRADING_CLOSED" << endl;
    ret = exchange_evt_dw->write(closed, exchange_handle);
    if (ret != DDS::RETCODE_OK) {
      ACE_ERROR ((LM_ERROR, ACE_TEXT("(%P|%t) ERROR: CLOSED write returned %d.\n"), ret));
    }

    cout << "Exiting..." << endl;
  } catch (CORBA::Exception& e) {
    cerr << "Exception caught in main.cpp:" << endl
         << e << endl;
    ACE_OS::exit(1);
  }

  // Cleanup
  try {
    if (!CORBA::is_nil (participant.in ())) {
      participant->delete_contained_entities();
    }
    if (!CORBA::is_nil (dpf.in ())) {
      dpf->delete_participant(participant.in ());
    }
  } catch (CORBA::Exception& e) {
    cerr << "Exception caught in cleanup." << endl << e << endl;
    ACE_OS::exit(1);
  }
  TheServiceParticipant->shutdown();
  return 0;
}
