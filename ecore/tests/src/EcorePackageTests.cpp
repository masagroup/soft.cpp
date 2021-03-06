#include <boost/test/unit_test.hpp>

#include "ecore/AnyCast.hpp"
#include "ecore/EAttribute.hpp"
#include "ecore/EClassifier.hpp"
#include "ecore/EList.hpp"
#include "ecore/EcoreFactory.hpp"
#include "ecore/EcorePackage.hpp"
#include "ecore/tests/MockEDataType.hpp"
#include "ecore/tests/MockEFactory.hpp"
#include "ecore/tests/MockEObjectInternal.hpp"

using namespace ecore;
using namespace ecore::tests;

BOOST_AUTO_TEST_SUITE( EcorePackageTests )

BOOST_AUTO_TEST_CASE( Constructor )
{
    BOOST_CHECK( EcorePackage::eInstance() );
}

BOOST_AUTO_TEST_CASE( Getters )
{
    BOOST_CHECK_EQUAL( EcorePackage::eInstance()->getEFactoryInstance(), EcoreFactory::eInstance() );
}

BOOST_AUTO_TEST_CASE( Package_Constructor )
{
    auto ecoreFactory = EcoreFactory::eInstance();
    auto ecorePackage = EcorePackage::eInstance();

    auto ePackage = ecoreFactory->createEPackage();
    BOOST_CHECK( ePackage );
}

BOOST_AUTO_TEST_CASE( Package_Accessors_Attributes )
{
    auto ecoreFactory = EcoreFactory::eInstance();
    auto ecorePackage = EcorePackage::eInstance();

    auto ePackage = ecoreFactory->createEPackage();
    ePackage->setName( "ePackageName" );
    BOOST_CHECK_EQUAL( ePackage->getName(), "ePackageName" );

    ePackage->setNsPrefix( "eNsPrefix" );
    BOOST_CHECK_EQUAL( ePackage->getNsPrefix(), "eNsPrefix" );

    ePackage->setNsURI( "eNsURI" );
    BOOST_CHECK_EQUAL( ePackage->getNsURI(), "eNsURI" );
}

BOOST_AUTO_TEST_CASE( Package_Accessors_FactoryInstance )
{
    auto ecoreFactory = EcoreFactory::eInstance();
    auto ecorePackage = EcorePackage::eInstance();

    auto ePackage = ecoreFactory->createEPackage();
    auto mockFactory = std::make_shared<MockEFactory>();
    auto mockInternal = std::make_shared<MockEObjectInternal>();
    MOCK_EXPECT( mockFactory->getInternal ).returns( *mockInternal );
    MOCK_EXPECT( mockInternal->eInverseAdd ).with( ePackage, EcorePackage::EFACTORY__EPACKAGE, nullptr ).returns( nullptr );
    ePackage->setEFactoryInstance( mockFactory );
    BOOST_CHECK_EQUAL( ePackage->getEFactoryInstance(), mockFactory );
}

BOOST_AUTO_TEST_CASE( Package_Accessors_Classifiers )
{
    auto ecoreFactory = EcoreFactory::eInstance();
    auto ecorePackage = EcorePackage::eInstance();

    auto ePackage = ecoreFactory->createEPackage();

    // add classifiers in the package
    auto eClassifier1 = std::make_shared<MockEClassifier>();
    auto eClassifier2 = std::make_shared<MockEClassifier>();
    auto eClassifier3 = std::make_shared<MockEClassifier>();
    auto mockInternal = std::make_shared<MockEObjectInternal>();

    MOCK_EXPECT( eClassifier1->getName ).returns( "eClassifier1" );
    MOCK_EXPECT( eClassifier1->getInternal ).returns( *mockInternal );
    MOCK_EXPECT( mockInternal->eInverseAdd ).with( ePackage, EcorePackage::ECLASSIFIER__EPACKAGE, nullptr ).returns( nullptr );
    MOCK_EXPECT( eClassifier2->getName ).returns( "eClassifier2" );
    MOCK_EXPECT( eClassifier2->getInternal ).returns( *mockInternal );
    MOCK_EXPECT( mockInternal->eInverseAdd ).with( ePackage, EcorePackage::ECLASSIFIER__EPACKAGE, nullptr ).returns( nullptr );
    MOCK_EXPECT( eClassifier3->getName ).returns( "eClassifier3" );
    MOCK_EXPECT( eClassifier3->getInternal ).returns( *mockInternal );
    MOCK_EXPECT( mockInternal->eInverseAdd ).with( ePackage, EcorePackage::ECLASSIFIER__EPACKAGE, nullptr ).returns( nullptr );
    ePackage->getEClassifiers()->add( eClassifier1 );
    ePackage->getEClassifiers()->add( eClassifier2 );

    // retrieve them in the package
    BOOST_CHECK_EQUAL( ePackage->getEClassifier( "eClassifier1" ), eClassifier1 );
    BOOST_CHECK_EQUAL( ePackage->getEClassifier( "eClassifier2" ), eClassifier2 );

    // ensure that even if we add it after getting previous ones , the cache inside
    // package is still valid
    ePackage->getEClassifiers()->add( eClassifier3 );
    BOOST_CHECK_EQUAL( ePackage->getEClassifier( "eClassifier3" ), eClassifier3 );
}

BOOST_AUTO_TEST_CASE( eGet_Reference )
{
    auto f = EcoreFactory::eInstance();

    auto a = f->createEAttribute();
    auto t = std::make_shared<MockEDataType>();
    MOCK_EXPECT( t->eIsProxy ).returns( true );
    a->setEType( t );

    BOOST_CHECK( toAny( t ) == a->eGet( EcorePackage::eInstance()->getETypedElement_EType(), false ) );    
}

BOOST_AUTO_TEST_SUITE_END()
