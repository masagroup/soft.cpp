// Code generated by soft.generator.cpp. DO NOT EDIT.

// *****************************************************************************
// Copyright(c) 2021 MASA Group
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// *****************************************************************************
#ifndef ECORE_EOBJECT_MOCKEOBJECT_HPP
#define ECORE_EOBJECT_MOCKEOBJECT_HPP


#include "ecore/EClass.hpp"
#include "ecore/EObject.hpp"
#include "ecore/EOperation.hpp"
#include "ecore/EReference.hpp"
#include "ecore/EStructuralFeature.hpp"
#include "ecore/tests/MockENotifier.hpp"

namespace ecore::tests
{

    template <typename I>
    class MockEObjectBase : public MockENotifierBase<I> 
    {
    public:
        typedef EObject base_type;
        
		MOCK_METHOD(eClass ,0); 
		MOCK_METHOD(eIsProxy ,0); 
		MOCK_METHOD(eResource ,0); 
		MOCK_METHOD(eContainer ,0); 
		MOCK_METHOD(eContainingFeature ,0); 
		MOCK_METHOD(eContainmentFeature ,0); 
		MOCK_METHOD(eContents ,0); 
		MOCK_METHOD(eAllContents ,0); 
		MOCK_METHOD(eCrossReferences ,0); 
		MOCK_METHOD_EXT(eGet ,1, Any(const std::shared_ptr<EStructuralFeature>&), eGet_EStructuralFeature); 
		MOCK_METHOD_EXT(eGet ,2, Any(const std::shared_ptr<EStructuralFeature>&,bool), eGet_EStructuralFeature_EBoolean); 
		MOCK_METHOD(eSet ,2); 
		MOCK_METHOD(eIsSet ,1); 
		MOCK_METHOD(eUnset ,1); 
		MOCK_METHOD(eInvoke ,2); 
		

        // Start of user code MockEObject
        MOCK_CONST_METHOD( getInternal, 0, impl::EObjectInternal&(), getInternalConst );
        MOCK_NON_CONST_METHOD( getInternal, 0, impl::EObjectInternal&(), getInternal );
        // End of user code
    };

    typedef MockEObjectBase<EObject> MockEObject;
} 

#endif /* ECORE_EOBJECT_MOCKEOBJECT_HPP */
