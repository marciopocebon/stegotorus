/* Copyright 2011, 2012, 2013 SRI International
 * See LICENSE for other credits and copying information
 */

#include "util.h"
#include "connections.h"
#include "protocol.h"
#include "steg.h"
#include <event2/buffer.h>

namespace {
  struct nosteg_steg_config_t : steg_config_t
  {
    STEG_CONFIG_DECLARE_METHODS(nosteg);
  };

  struct nosteg_steg_t : steg_t
  {
    nosteg_steg_config_t *config;
    conn_t *conn;

    nosteg_steg_t(nosteg_steg_config_t *cf, conn_t *cn);

    STEG_DECLARE_METHODS(nosteg);

    DISALLOW_COPY_AND_ASSIGN(nosteg_steg_t);
    
  };
}

STEG_DEFINE_MODULE(nosteg);

nosteg_steg_config_t::nosteg_steg_config_t(config_t *cfg)
  : steg_config_t(cfg)
{
}

nosteg_steg_config_t::~nosteg_steg_config_t()
{
}

steg_t *
nosteg_steg_config_t::steg_create(conn_t *conn)
{
  return new nosteg_steg_t(this, conn);
}

nosteg_steg_t::nosteg_steg_t(nosteg_steg_config_t *cf, conn_t *cn)
  : config(cf), conn(cn)
{
}

nosteg_steg_t::~nosteg_steg_t()
{
}

steg_config_t *
nosteg_steg_t::cfg()
{
  return config;
}


void
nosteg_steg_t::successful_reception() {};

unsigned int
nosteg_steg_t::corrupted_reception() {return 0;};


size_t
nosteg_steg_t::transmit_room(size_t pref, size_t, size_t)
{
  return pref;
}

transmit_t
nosteg_steg_t::transmit(struct evbuffer *source)
{
  struct evbuffer *dest = conn->outbound();

  log_debug(conn, "transmitting %lu bytes",
            (unsigned long)evbuffer_get_length(source));

  if (evbuffer_add_buffer(dest, source)) {
    log_warn(conn, "failed to transfer buffer");
    return NOT_TRANSMITTED;
  }

  return TRANSMIT_GOOD;
}

recv_t
nosteg_steg_t::receive(struct evbuffer *dest)
{
  struct evbuffer *source = conn->inbound();

  log_debug(conn, "receiving %lu bytes",
            (unsigned long)evbuffer_get_length(source));

  if (evbuffer_add_buffer(dest, source)) {
    log_warn(conn, "failed to transfer buffer");
    return RECV_BAD;
  }

  return RECV_GOOD;
}