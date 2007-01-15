/*
 * Copyright (C) 2007 Michael Brown <mbrown@fensystems.co.uk>.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/**
 * @file
 *
 * Fetch file as executable/loadable image
 *
 */

#include <errno.h>
#include <vsprintf.h>
#include <gpxe/emalloc.h>
#include <gpxe/ebuffer.h>
#include <gpxe/image.h>
#include <gpxe/uri.h>
#include <usr/fetch.h>

#include <byteswap.h>
#include <gpxe/dhcp.h>
#include <gpxe/tftp.h>
#include <gpxe/http.h>

/**
 * Fetch file
 *
 * @v filename		Filename to fetch
 * @ret data		Loaded file
 * @ret len		Length of loaded file
 * @ret rc		Return status code
 *
 * Fetch file to an external buffer allocated with emalloc().  The
 * caller is responsible for eventually freeing the buffer with
 * efree().
 */
int fetch ( const char *uri_string, userptr_t *data, size_t *len ) {
	struct uri *uri;
	struct buffer buffer;
	int rc;

	/* Parse the URI */
	uri = parse_uri ( uri_string );
	if ( ! uri ) {
		rc = -ENOMEM;
		goto err_parse_uri;
	}

	/* Allocate an expandable buffer to hold the file */
	if ( ( rc = ebuffer_alloc ( &buffer, 0 ) ) != 0 ) {
		goto err_ebuffer_alloc;
	}

#warning "Temporary pseudo-URL parsing code"

	/* Retrieve the file */
	struct async async;

	int ( * download ) ( struct uri *uri, struct buffer *buffer,
			     struct async *parent );

#if 0
	server.sin.sin_port = htons ( TFTP_PORT );
	udp_connect ( &tftp.udp, &server.st );
	tftp.filename = filename;
	tftp.buffer = &buffer;
	aop = tftp_get ( &tftp );
#else
	download = http_get;
#endif

	async_init_orphan ( &async );
	if ( ( rc = download ( uri, &buffer, &async ) ) != 0 )
		goto err;
	uri = NULL;
	async_wait ( &async, &rc, 1 );
	if ( rc != 0 )
		goto err;

	/* Fill in buffer address and length */
	*data = buffer.addr;
	*len = buffer.fill;

	/* Release temporary resources.  The ebuffer storage is now
	 * owned by our caller, so we don't free it.
	 */

	return 0;

 err:
	efree ( buffer.addr );
 err_ebuffer_alloc:
	free_uri ( uri );
 err_parse_uri:
	return rc;
}
