// *****************************************************************************
//
// This file is part of a MASA library or program.
// Refer to the included end-user license agreement for restrictions.
//
// Copyright (c) 2020 MASA Group
//
// *****************************************************************************

#ifndef ECORE_BASICEOBJECT_HPP_
#error This file may only be included from BasicEObject.hpp
#endif

#include "ecore/Any.hpp"
#include "ecore/AnyCast.hpp"
#include "ecore/Constants.hpp"
#include "ecore/EAdapter.hpp"
#include "ecore/EClass.hpp"
#include "ecore/ECollectionView.hpp"
#include "ecore/ENotifyingList.hpp"
#include "ecore/EOperation.hpp"
#include "ecore/EReference.hpp"
#include "ecore/EResource.hpp"
#include "ecore/EStructuralFeature.hpp"
#include "ecore/EcorePackage.hpp"
#include "ecore/EcoreUtils.hpp"
#include "ecore/impl/AbstractAdapter.hpp"
#include "ecore/impl/ImmutableArrayEList.hpp"
#include "ecore/impl/Notification.hpp"

#include <sstream>
#include <string>

namespace ecore::impl
{
    template <typename... I>
    class BasicEObject<I...>::EContentsEList : public AbstractAdapter
    {
        typedef std::shared_ptr<const EList<std::shared_ptr<ecore::EStructuralFeature>>> ( EClass::*T_FeaturesGetter )() const;

    public:
        EContentsEList( BasicEObject<I...>& obj, T_FeaturesGetter featuresGetter )
            : obj_( obj )
            , featuresGetter_( featuresGetter )
        {
            obj_.eAdapters().add( this );
        }

        virtual ~EContentsEList()
        {
            obj_.eAdapters().remove( this );
        }

        virtual void notifyChanged( const std::shared_ptr<ENotification>& notification )
        {
            // if we have a list && the event is not removing this adapter from the object
            if( l_ && !isThisAdapterRemoved( notification ) )
            {
                auto feature = notification->getFeature();
                auto features = std::invoke( featuresGetter_, *obj_.eClass() );
                if( features->contains( feature ) )
                    l_.reset();
            }
        }

        std::shared_ptr<const EList<std::shared_ptr<EObject>>> getList()
        {
            // AbstractEContentsEList implements a lazy list that computes
            // list of contents only when needed
            // this list can be resolved/unresolved
            class AbstractEContentsEList : public ImmutableArrayEList<std::shared_ptr<EObject>>
            {
            public:
                typedef ImmutableArrayEList<std::shared_ptr<EObject>> Super;

                AbstractEContentsEList( const EContentsEList& l, bool resolve )
                    : l_( l )
                    , resolve_( resolve )
                    , initialized_( false )
                {
                }

                virtual ~AbstractEContentsEList() = default;

                virtual std::shared_ptr<EObject> get( std::size_t pos ) const
                {
                    const_cast<AbstractEContentsEList*>( this )->initialize();
                    return Super::get( pos );
                }

                virtual std::size_t size() const
                {
                    const_cast<AbstractEContentsEList*>( this )->initialize();
                    return Super::size();
                }

                virtual bool empty() const
                {
                    const_cast<AbstractEContentsEList*>( this )->initialize();
                    return Super::empty();
                }

                virtual bool contains( const std::shared_ptr<EObject>& e ) const
                {
                    const_cast<AbstractEContentsEList*>( this )->initialize();
                    return Super::contains( e );
                }

                virtual std::size_t indexOf( const std::shared_ptr<EObject>& e ) const
                {
                    const_cast<AbstractEContentsEList*>( this )->initialize();
                    return Super::indexOf( e );
                }

            private:
                void initialize()
                {
                    if( initialized_ )
                        return;

                    initialized_ = true;
                    const auto& o = l_.obj_;
                    auto features = std::invoke( l_.featuresGetter_, *o.eClass() );
                    for( auto feature : features )
                    {
                        if( o.eIsSet( feature ) )
                        {
                            auto value = o.eGet( feature, resolve_ );
                            if( feature->isMany() )
                            {
                                auto l = anyListCast<std::shared_ptr<EObject>>( value );
                                v_.reserve( v_.size() + l->size() );
                                std::copy( l->begin(), l->end(), std::back_inserter( v_ ) );
                            }
                            else if( !value.empty() )
                            {
                                auto object = anyObjectCast<std::shared_ptr<EObject>>( value );
                                if( object )
                                    v_.push_back( object );
                            }
                        }
                    }
                }

            protected:
                const EContentsEList& l_;

            private:
                bool resolve_;
                bool initialized_;
            };

            // UnResolvedEContentsEList implements a lazy list that computes
            // list of unresolved contents only when needed
            class UnResolvedEContentsEList : public AbstractEContentsEList
            {
            public:
                UnResolvedEContentsEList( const EContentsEList& l )
                    : AbstractEContentsEList( l, false )
                {
                }

                virtual ~UnResolvedEContentsEList() = default;
            };

            // ResolvedEContentsEList implements a lazy list that computes
            // list of resolved contents only when needed
            class ResolvedEContentsEList : public AbstractEContentsEList
            {
            public:
                ResolvedEContentsEList( const EContentsEList& l )
                    : AbstractEContentsEList( l, true )
                {
                }

                virtual ~ResolvedEContentsEList() = default;

                virtual std::shared_ptr<const EList<std::shared_ptr<EObject>>> getUnResolvedList() const
                {
                    return std::make_shared<UnResolvedEContentsEList>( l_ );
                }
            };

            // the real list
            if( !l_ )
                l_ = std::make_shared<ResolvedEContentsEList>( *this );
            return l_;
        }

    private:
        inline bool isThisAdapterRemoved( const std::shared_ptr<ENotification>& notification ) const
        {
            return notification->getEventType() == Notification::REMOVING_ADAPTER
                   && anyCast<EAdapter*>( notification->getOldValue() ) == this;
        }

    private:
        BasicEObject<I...>& obj_;
        T_FeaturesGetter featuresGetter_;
        std::shared_ptr<const EList<std::shared_ptr<EObject>>> l_;
    };

    template <typename... I>
    BasicEObject<I...>::BasicEObject()
        : eContainer_()
        , eContainerFeatureID_( -1 )
    {
    }

    template <typename... I>
    BasicEObject<I...>::~BasicEObject()
    {
    }

    template <typename... I>
    const EObjectInternal& BasicEObject<I...>::getInternal() const
    {
        return const_cast<BasicEObject<I...>*>( this )->getInternal();
    }

    template <typename... I>
    EObjectInternal& BasicEObject<I...>::getInternal()
    {
        if( !internal_ )
            internal_ = std::move( createInternal() );
        return *internal_;
    }

    template <typename... I>
    std::shared_ptr<const ECollectionView<std::shared_ptr<ecore::EObject>>> BasicEObject<I...>::eAllContents() const
    {
        return std::make_shared<ECollectionView<std::shared_ptr<ecore::EObject>>>( eContents() );
    }

    template <typename... I>
    std::shared_ptr<const EList<std::shared_ptr<ecore::EObject>>> BasicEObject<I...>::eContents() const
    {
        return const_cast<BasicEObject<I...>*>( this )->eContentsList();
    }

    template <typename... I>
    std::shared_ptr<const EList<std::shared_ptr<ecore::EObject>>> BasicEObject<I...>::eCrossReferences() const
    {
        return const_cast<BasicEObject<I...>*>( this )->eCrossReferencesList();
    }

    template <typename... I>
    typename std::shared_ptr<const EList<std::shared_ptr<EObject>>> BasicEObject<I...>::eContentsList()
    {
        if( !eContents_ )
            eContents_ = std::make_unique<EContentsEList>( *this, &EClass::getEContainmentFeatures );
        return eContents_->getList();
    }

    template <typename... I>
    typename std::shared_ptr<const EList<std::shared_ptr<EObject>>> BasicEObject<I...>::eCrossReferencesList()
    {
        if( !eCrossReferences_ )
            eCrossReferences_ = std::make_unique<EContentsEList>( *this, &EClass::getECrossReferenceFeatures );
        return eCrossReferences_->getList();
    }

    template <typename... I>
    std::shared_ptr<ecore::EClass> BasicEObject<I...>::eClass() const
    {
        return eStaticClass();
    }

    template <typename... I>
    std::shared_ptr<EClass> BasicEObject<I...>::eStaticClass() const
    {
        return EcorePackage::eInstance()->getEObject();
    }

    template <typename... I>
    std::shared_ptr<EObject> BasicEObject<I...>::eObjectForFragmentSegment( const std::string& uriSegment ) const
    {
        std::size_t index = std::string::npos;
        if( !uriSegment.empty() && std::isdigit( uriSegment.back() ) )
        {
            index = uriSegment.find_last_of( '.' );
            if( index != std::string::npos )
            {
                auto position = std::stoi( uriSegment.substr( index + 1 ) );
                auto eFeatureName = uriSegment.substr( 1, index - 1 );
                auto eFeature = eStructuralFeature( eFeatureName );
                auto value = eGet( eFeature, false );
                auto list = anyListCast<std::shared_ptr<EObject>>( value );
                if( position < list->size() )
                    return list->get( position );
            }
        }
        if( index == std::string::npos )
        {
            auto eFeature = eStructuralFeature( uriSegment.substr( 1 ) );
            auto value = eGet( eFeature, false );
            return anyCast<std::shared_ptr<EObject>>( value );
        }
        return std::shared_ptr<EObject>();
    }

    template <typename... I>
    std::string BasicEObject<I...>::eURIFragmentSegment( const std::shared_ptr<EStructuralFeature>& eFeature,
                                                         const std::shared_ptr<EObject>& eObject ) const
    {
        std::stringstream s;
        s << "@";
        s << eFeature->getName();
        if( eFeature->isMany() )
        {
            auto v = eGet( eFeature, false );
            auto l = anyListCast<std::shared_ptr<EObject>>( v );
            auto index = l->indexOf( eObject );
            s << ".";
            s << index;
        }
        return s.str();
    }

    template <typename... I>
    inline std::shared_ptr<EObject> BasicEObject<I...>::getThisAsEObject() const
    {
        return std::static_pointer_cast<EObject>( getThisPtr() );
    }

    template <typename... I>
    std::shared_ptr<EStructuralFeature> BasicEObject<I...>::eStructuralFeature( const std::string& name ) const
    {
        auto eFeature = eClass()->getEStructuralFeature( name );
        if( !eFeature )
            throw std::runtime_error( "The feature " + name + " is not a valid feature" );
        return eFeature;
    }

    template <typename... I>
    std::shared_ptr<ecore::EStructuralFeature> BasicEObject<I...>::eContainingFeature() const
    {
        auto eContainer = eContainer_.lock();
        if( eContainer )
        {
            return eContainerFeatureID_ <= EOPPOSITE_FEATURE_BASE
                       ? eContainer->eClass()->getEStructuralFeature( EOPPOSITE_FEATURE_BASE - eContainerFeatureID_ )
                       : std::static_pointer_cast<EReference>( eClass()->getEStructuralFeature( eContainerFeatureID_ ) )->getEOpposite();
        }
        return std::shared_ptr<ecore::EStructuralFeature>();
    }

    template <typename... I>
    std::shared_ptr<ecore::EReference> BasicEObject<I...>::eContainmentFeature() const
    {
        return eContainmentFeature( getThisAsEObject(), eContainer_.lock(), eContainerFeatureID_ );
    }

    template <typename... I>
    std::shared_ptr<EReference> BasicEObject<I...>::eContainmentFeature( const std::shared_ptr<EObject>& eObject,
                                                                         const std::shared_ptr<EObject>& eContainer,
                                                                         int eContainerFeatureID )
    {
        if( eContainer )
        {
            if( eContainerFeatureID <= EOPPOSITE_FEATURE_BASE )
            {
                auto eFeature = eContainer->eClass()->getEStructuralFeature( EOPPOSITE_FEATURE_BASE - eContainerFeatureID );
                if( auto eReference = std::dynamic_pointer_cast<EReference>( eFeature ) )
                    return eReference;
            }
            else
            {
                auto eFeature = eObject->eClass()->getEStructuralFeature( eContainerFeatureID );
                if( auto eReference = std::dynamic_pointer_cast<EReference>( eFeature ) )
                    return eReference->getEOpposite();
            }
            throw "The containment feature could not be located";
        }
        return std::shared_ptr<EReference>();
    }

    template <typename... I>
    bool BasicEObject<I...>::eIsProxy() const
    {
        return static_cast<bool>( eProxyURI_ );
    }

    template <typename... I>
    std::shared_ptr<EResource> BasicEObject<I...>::eResource() const
    {
        auto eResource = eResource_.lock();
        if( !eResource )
        {
            auto eContainer = eContainer_.lock();
            if( eContainer )
                eResource = eContainer->eResource();
        }
        return eResource;
    }

    template <typename... I>
    std::shared_ptr<EResource> BasicEObject<I...>::eInternalResource() const
    {
        return eResource_.lock();
    }

    template <typename... I>
    void BasicEObject<I...>::eSetInternalResource( const std::shared_ptr<EResource>& resource )
    {
        eResource_ = resource;
    }

    template <typename... I>
    std::shared_ptr<ENotificationChain> BasicEObject<I...>::eSetInternalResource( const std::shared_ptr<EResource>& newResource,
                                                                                  const std::shared_ptr<ENotificationChain>& n )
    {
        auto notifications = n;
        auto oldResource = eResource_.lock();
        auto thisObject = getThisAsEObject();
        if( oldResource && newResource )
        {
            auto list = std::static_pointer_cast<ENotifyingList<std::shared_ptr<EObject>>>( oldResource->getContents() );
            notifications = list->remove( thisObject, notifications );

            oldResource->detached( thisObject );
        }

        auto eContainer = eContainer_.lock();
        if( eContainer )
        {
            if( eContainmentFeature()->isResolveProxies() )
            {
                auto oldContainerResource = eContainer->eResource();
                if( oldContainerResource )
                {
                    // If we're not setting a new resource, attach it to the old container's resource.
                    if( !newResource )
                        oldContainerResource->attached( thisObject );
                    // If we didn't detach it from an old resource already, detach it from the old container's resource.
                    else if( !oldResource )
                        oldContainerResource->detached( thisObject );
                }
            }
            else
            {
                notifications = eBasicRemoveFromContainer( notifications );
                notifications = eBasicSetContainer( nullptr, -1, notifications );
            }
        }
        eSetInternalResource( newResource );
        return notifications;
    }

    template <typename... I>
    Any BasicEObject<I...>::eGet( const std::shared_ptr<EStructuralFeature>& feature ) const
    {
        return eGet( feature, true );
    }

    template <typename... I>
    Any BasicEObject<I...>::eGet( const std::shared_ptr<EStructuralFeature>& feature, bool resolve ) const
    {
        int featureID = eStructuralFeatureID( feature );
        if( featureID >= 0 )
            return eGet( featureID, resolve );
        throw "The feature '" + feature->getName() + "' is not a valid feature";
    }

    template <typename... I>
    int BasicEObject<I...>::eStructuralFeatureID( const std::shared_ptr<EStructuralFeature>& eStructuralFeature ) const
    {
        VERIFYN( eClass()->getEAllStructuralFeatures()->contains( eStructuralFeature ),
                 "The feature '%s' is not a valid feature",
                 eStructuralFeature->getName().c_str() );
        return eStructuralFeatureID( eStructuralFeature->eContainer(), eStructuralFeature->getFeatureID() );
    }

    template <typename... I>
    int BasicEObject<I...>::eStructuralFeatureID( const std::shared_ptr<EObject>& eContainer, int featureID ) const
    {
        return featureID;
    }

    template <typename... I>
    int BasicEObject<I...>::eOperationID( const std::shared_ptr<EOperation>& eOperation ) const
    {
        VERIFYN( eClass()->getEAllOperations()->contains( eOperation ),
                 "The operation '%s' is not a valid operation",
                 eOperation->getName().c_str() );
        return eOperationID( eOperation->eContainer(), eOperation->getOperationID() );
    }

    template <typename... I>
    int BasicEObject<I...>::eOperationID( const std::shared_ptr<EObject>& eContainer, int operationID ) const
    {
        return operationID;
    }

    template <typename... I>
    Any BasicEObject<I...>::eGet( int featureID, bool resolve ) const
    {
        std::shared_ptr<EStructuralFeature> eFeature = eClass()->getEStructuralFeature( featureID );
        VERIFYN( eFeature, "Invalid featureID: %i ", featureID );
        return Any();
    }

    template <typename... I>
    bool BasicEObject<I...>::eIsSet( const std::shared_ptr<EStructuralFeature>& eFeature ) const
    {
        int featureID = eStructuralFeatureID( eFeature );
        if( featureID >= 0 )
            return eIsSet( featureID );
        throw "The feature '" + eFeature->getName() + "' is not a valid feature";
    }

    template <typename... I>
    bool BasicEObject<I...>::eIsSet( int featureID ) const
    {
        std::shared_ptr<EStructuralFeature> eFeature = eClass()->getEStructuralFeature( featureID );
        VERIFYN( eFeature, "Invalid featureID: %i ", featureID );
        return false;
    }

    template <typename... I>
    void BasicEObject<I...>::eSet( const std::shared_ptr<EStructuralFeature>& eFeature, const Any& newValue )
    {
        int featureID = eStructuralFeatureID( eFeature );
        if( featureID >= 0 )
            eSet( featureID, newValue );
        else
            throw "The feature '" + eFeature->getName() + "' is not a valid feature";
    }

    template <typename... I>
    void BasicEObject<I...>::eSet( int featureID, const Any& newValue )
    {
        std::shared_ptr<EStructuralFeature> eFeature = eClass()->getEStructuralFeature( featureID );
        VERIFYN( eFeature, "Invalid featureID: %i ", featureID );
    }

    template <typename... I>
    void BasicEObject<I...>::eUnset( const std::shared_ptr<EStructuralFeature>& eFeature )
    {
        int featureID = eStructuralFeatureID( eFeature );
        if( featureID >= 0 )
            eUnset( featureID );
        else
            throw "The feature '" + eFeature->getName() + "' is not a valid feature";
    }

    template <typename... I>
    void BasicEObject<I...>::eUnset( int featureID )
    {
        std::shared_ptr<EStructuralFeature> eFeature = eClass()->getEStructuralFeature( featureID );
        VERIFYN( eFeature, "Invalid featureID: %i ", featureID );
    }

    template <typename... I>
    Any BasicEObject<I...>::eInvoke( const std::shared_ptr<EOperation>& eOperation, const std::shared_ptr<EList<Any>>& arguments )
    {
        int operationID = eOperationID( eOperation );
        if( operationID >= 0 )
            return eInvoke( operationID, arguments );
        throw "The operation '" + eOperation->getName() + "' is not a valid operation";
    }

    template <typename... I>
    Any BasicEObject<I...>::eInvoke( int operationID, const std::shared_ptr<EList<Any>>& arguments )
    {
        std::shared_ptr<EOperation> eOperation = eClass()->getEOperation( operationID );
        VERIFYN( eOperation, "Invalid operationID: %i ", operationID );
        return Any();
    }

    template <typename... I>
    std::shared_ptr<ENotificationChain> BasicEObject<I...>::eBasicInverseAdd( const std::shared_ptr<EObject>& otherEnd,
                                                                              int featureID,
                                                                              const std::shared_ptr<ENotificationChain>& notifications )
    {
        return notifications;
    }

    template <typename... I>
    std::shared_ptr<ENotificationChain> BasicEObject<I...>::eBasicInverseRemove( const std::shared_ptr<EObject>& otherEnd,
                                                                                 int featureID,
                                                                                 const std::shared_ptr<ENotificationChain>& notifications )
    {
        return notifications;
    }

    template <typename... I>
    std::shared_ptr<ENotificationChain> BasicEObject<I...>::eInverseAdd( const std::shared_ptr<EObject>& otherEnd,
                                                                         int featureID,
                                                                         const std::shared_ptr<ENotificationChain>& n )
    {
        auto notifications = n;
        if( featureID >= 0 )
            return eBasicInverseAdd( otherEnd, featureID, notifications );
        else
        {
            notifications = eBasicRemoveFromContainer( notifications );
            return eBasicSetContainer( otherEnd, featureID, notifications );
        }
    }

    template <typename... I>
    std::shared_ptr<ENotificationChain> BasicEObject<I...>::eInverseRemove( const std::shared_ptr<EObject>& otherEnd,
                                                                            int featureID,
                                                                            const std::shared_ptr<ENotificationChain>& notifications )
    {
        if( featureID >= 0 )
            return eBasicInverseRemove( otherEnd, featureID, notifications );
        else
            return eBasicSetContainer( nullptr, featureID, notifications );
    }

    template <typename... I>
    URI BasicEObject<I...>::eProxyURI() const
    {
        return eProxyURI_.value_or( URI() );
    }

    template <typename... I>
    void BasicEObject<I...>::eSetProxyURI( const URI& uri )
    {
        eProxyURI_ = uri;
    }

    template <typename... I>
    std::shared_ptr<EObject> BasicEObject<I...>::eResolveProxy( const std::shared_ptr<EObject>& proxy ) const
    {
        return EcoreUtils::resolve( proxy, getThisAsEObject() );
    }

    template <typename... I>
    std::shared_ptr<ecore::EObject> BasicEObject<I...>::eContainer() const
    {
        return const_cast<BasicEObject<I...>*>( this )->eContainer();
    }

    template <typename... I>
    std::shared_ptr<EObject> BasicEObject<I...>::eContainer()
    {
        auto eContainer = eContainer_.lock();
        if( eContainer && eContainer->eIsProxy() )
        {
            auto resolved = eResolveProxy( eContainer );
            if( resolved != eContainer )
            {
                auto notifications = eBasicRemoveFromContainer( nullptr );
                eBasicSetContainer( resolved, eContainerFeatureID_ );
                if( notifications )
                    notifications->dispatch();
                if( eNotificationRequired() && eContainerFeatureID_ >= EOPPOSITE_FEATURE_BASE )
                    eNotify( std::make_shared<Notification>(
                        getThisAsEObject(), Notification::RESOLVE, eContainerFeatureID_, eContainer, resolved ) );
            }
            return resolved;
        }
        return eContainer;
    }

    template <typename... I>
    std::shared_ptr<EObject> BasicEObject<I...>::eInternalContainer() const
    {
        return eContainer_.lock();
    }

    template <typename... I>
    int BasicEObject<I...>::eContainerFeatureID() const
    {
        return eContainerFeatureID_;
    }

    template <typename... I>
    inline void BasicEObject<I...>::eBasicSetContainer( const std::shared_ptr<EObject>& newContainer, int newContainerFeatureID )
    {
        eContainer_ = newContainer;
        eContainerFeatureID_ = newContainerFeatureID;
    }

    template <typename... I>
    std::shared_ptr<ENotificationChain> BasicEObject<I...>::eBasicSetContainer( const std::shared_ptr<EObject>& newContainer,
                                                                                int newContainerFeatureID,
                                                                                const std::shared_ptr<ENotificationChain>& n )
    {
        auto notifications = n;
        auto oldContainer = eContainer_.lock();
        auto oldResource = eResource_.lock();
        auto thisObject = getThisAsEObject();

        // resource
        std::shared_ptr<EResource> newResource;
        if( oldResource )
        {
            if( newContainer && !eContainmentFeature( thisObject, newContainer, newContainerFeatureID )->isResolveProxies() )
            {
                auto list = std::static_pointer_cast<ENotifyingList<std::shared_ptr<EObject>>>( oldResource->getContents() );
                notifications = list->remove( thisObject, notifications );
                eSetInternalResource( nullptr );
                newResource = newContainer->eResource();
            }
            else
                oldResource = nullptr;
        }
        else
        {
            if( oldContainer )
                oldResource = oldContainer->eResource();

            if( newContainer )
                newResource = newContainer->eResource();
        }

        if( oldResource && oldResource != newResource )
            oldResource->detached( thisObject );

        if( newResource && newResource != oldResource )
            newResource->attached( thisObject );

        // basic set
        int oldContainerFeatureID = eContainerFeatureID_;
        eBasicSetContainer( newContainer, newContainerFeatureID );

        // notification
        if( eNotificationRequired() )
        {
            if( oldContainer && oldContainerFeatureID >= 0 && oldContainerFeatureID != newContainerFeatureID )
            {
                auto notification = std::make_shared<Notification>(
                    thisObject, ENotification::SET, oldContainerFeatureID, oldContainer, std::shared_ptr<EObject>() );
                if( notifications )
                    notifications->add( notification );
                else
                    notifications = notification;
            }
            if( newContainerFeatureID >= 0 )
            {
                auto notification = std::make_shared<Notification>(
                    thisObject,
                    ENotification::SET,
                    newContainerFeatureID,
                    oldContainerFeatureID == newContainerFeatureID ? oldContainer : std::shared_ptr<EObject>(),
                    newContainer );
                if( notifications )
                    notifications->add( notification );
                else
                    notifications = notification;
            }
        }
        return notifications;
    }

    template <typename... I>
    std::shared_ptr<ENotificationChain> BasicEObject<I...>::eBasicRemoveFromContainer(
        const std::shared_ptr<ENotificationChain>& notifications )
    {
        if( eContainerFeatureID_ >= 0 )
            return eBasicRemoveFromContainerFeature( notifications );
        else
        {
            auto eContainer = eContainer_.lock();
            if( eContainer )
                return eContainer->getInternal().eInverseRemove(
                    getThisAsEObject(), EOPPOSITE_FEATURE_BASE - eContainerFeatureID_, notifications );
        }
        return notifications;
    }

    template <typename... I>
    std::shared_ptr<ENotificationChain> BasicEObject<I...>::eBasicRemoveFromContainerFeature(
        const std::shared_ptr<ENotificationChain>& notifications )
    {
        auto reference = std::dynamic_pointer_cast<EReference>( eClass()->getEStructuralFeature( eContainerFeatureID_ ) );
        if( reference )
        {
            auto inverseFeature = reference->getEOpposite();
            auto container = eContainer_.lock();
            if( container && inverseFeature )
                return container->getInternal().eInverseRemove( getThisAsEObject(), inverseFeature->getFeatureID(), notifications );
        }
        return notifications;
    }

    template <typename... I>
    template <typename U>
    class BasicEObject<I...>::EObjectInternalAdapter : public U
    {
    public:
        EObjectInternalAdapter( BasicEObject<I...>& obj )
            : obj_( obj )
        {
        }

        // Inherited via EObjectInternal
        virtual std::shared_ptr<EResource> eInternalResource() const override
        {
            return getObject().eInternalResource();
        }
        virtual std::shared_ptr<ENotificationChain> eSetInternalResource(
            const std::shared_ptr<EResource>& resource, const std::shared_ptr<ENotificationChain>& notifications ) override
        {
            return getObject().eSetInternalResource( resource, notifications );
        }
        virtual std::shared_ptr<EObject> eInternalContainer() const
        {
            return getObject().eInternalContainer();
        }
        virtual std::shared_ptr<EObject> eObjectForFragmentSegment( const std::string& uriSegment ) const override
        {
            return getObject().eObjectForFragmentSegment( uriSegment );
        }
        virtual std::string eURIFragmentSegment( const std::shared_ptr<EStructuralFeature>& feature,
                                                 const std::shared_ptr<EObject>& eObject ) const override
        {
            return getObject().eURIFragmentSegment( feature, eObject );
        }
        virtual std::shared_ptr<ENotificationChain> eInverseAdd( const std::shared_ptr<EObject>& otherEnd,
                                                                 int featureID,
                                                                 const std::shared_ptr<ENotificationChain>& notifications ) override
        {
            return getObject().eInverseAdd( otherEnd, featureID, notifications );
        }
        virtual std::shared_ptr<ENotificationChain> eInverseRemove( const std::shared_ptr<EObject>& otherEnd,
                                                                    int featureID,
                                                                    const std::shared_ptr<ENotificationChain>& notifications ) override
        {
            return getObject().eInverseRemove( otherEnd, featureID, notifications );
        }
        virtual URI eProxyURI() const override
        {
            return getObject().eProxyURI();
        }
        virtual void eSetProxyURI( const URI& uri ) override
        {
            return getObject().eSetProxyURI( uri );
        }
        virtual std::shared_ptr<EObject> eResolveProxy( const std::shared_ptr<EObject>& proxy ) const override
        {
            return getObject().eResolveProxy( proxy );
        }

        inline BasicEObject<I...>& getObject()
        {
            return obj_;
        }

        inline const BasicEObject<I...>& getObject() const
        {
            return obj_;
        }

    private:
        BasicEObject<I...>& obj_;
    };

    template <typename... I>
    std::unique_ptr<EObjectInternal> BasicEObject<I...>::createInternal()
    {
        return std::move( std::make_unique<EObjectInternalAdapter<EObjectInternal>>( *this ) );
    }

} // namespace ecore::impl