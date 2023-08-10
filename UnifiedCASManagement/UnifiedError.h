
#ifndef UNIFIEDERROR_H
#define UNIFIEDERROR_H

#define RTERROR_TO_WPEERROR(_err)       (_err == RT_OK)? Core::ERROR_NONE: Core::ERROR_GENERAL
#define RTERROR_TO_RESPONSE(_err)                           \
        do {                                                \
            response.Success = (_err == RT_OK);             \
            if (_err != RT_OK) {                            \
                response.Failurereason = (_err);    \
            }                                               \
        } while(0);

#endif /* UNIFIEDERROR_H */

