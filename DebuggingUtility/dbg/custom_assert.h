#pragma once
#define BOOST_ENABLE_ASSERT_DEBUG_HANDLER
#include <boost/assert.hpp>
#include <boost/static_assert.hpp>
#include "logging.h"

// TODO: DOOBIUS_ASSERT (have #expr and __debugbreak())
// TODO: DOOBIUS_DASSERT (DOOBIUS_ASSERT but only in Debug otherwise Null)
// TODO: 

#define DOOBIUS_ASSERT