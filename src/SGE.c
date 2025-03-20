//
// Created by Geisthardt on 17.03.2025.
//

#include "SGE.h"

SGE_VERSION get_sge_version() {
        const SGE_VERSION current_version = {
                .major = SGE_VERSION_MAJOR,
                .minor = SGE_VERSION_MINOR,
                .patch = SGE_VERSION_PATCH
        };

        return current_version;
}