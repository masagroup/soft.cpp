#include <boost/test/unit_test.hpp>

#include "ecore/impl/NotificationChain.hpp"
#include "ecore/tests/MockENotification.hpp"
#include "ecore/tests/MockENotifier.hpp"
#include "ecore/tests/MockEStructuralFeature.hpp"

using namespace ecore;
using namespace ecore::impl;
using namespace ecore::tests;

BOOST_AUTO_TEST_SUITE( NotificationChainTests )

BOOST_AUTO_TEST_CASE( Constructor )
{
    BOOST_CHECK_NO_THROW( std::make_shared<NotificationChain>() );
}

BOOST_AUTO_TEST_CASE( AddAndDispatch )
{
    auto notifier = std::make_shared<MockENotifier>();
    auto feature = std::make_shared<MockEStructuralFeature>();

    auto notification1 = std::make_shared<MockENotification>();
    MOCK_EXPECT( notification1->getEventType ).returns( ENotification::ADD );
    MOCK_EXPECT( notification1->getNotifier ).returns( notifier );

    auto notification2 = std::make_shared<MockENotification>();
    MOCK_EXPECT( notification2->getEventType ).returns( ENotification::ADD );
    MOCK_EXPECT( notification2->getNotifier ).returns( notifier );

    auto chain = std::make_shared<NotificationChain>();
    BOOST_CHECK( chain->add( notification1 ) );
    MOCK_EXPECT( notification1->merge ).with( notification2 ).returns( false );
    BOOST_CHECK( chain->add( notification2 ) );
    MOCK_EXPECT( notifier->eNotify ).with( notification1 );
    MOCK_EXPECT( notifier->eNotify ).with( notification2 );
    chain->dispatch();
}

BOOST_AUTO_TEST_SUITE_END()
