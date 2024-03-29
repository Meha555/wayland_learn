#include "qwayland-server-xdg-shell.h"

QT_BEGIN_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wmissing-field-initializers")

namespace QtWaylandServer {
    xdg_wm_base::xdg_wm_base(struct ::wl_client *client, int id, int version)
        : m_resource_map()
        , m_resource(nullptr)
        , m_global(nullptr)
    {
        init(client, id, version);
    }

    xdg_wm_base::xdg_wm_base(struct ::wl_display *display, int version)
        : m_resource_map()
        , m_resource(nullptr)
        , m_global(nullptr)
    {
        init(display, version);
    }

    xdg_wm_base::xdg_wm_base(struct ::wl_resource *resource)
        : m_resource_map()
        , m_resource(nullptr)
        , m_global(nullptr)
    {
        init(resource);
    }

    xdg_wm_base::xdg_wm_base()
        : m_resource_map()
        , m_resource(nullptr)
        , m_global(nullptr)
    {
    }

    xdg_wm_base::~xdg_wm_base()
    {
        for (auto resource : qAsConst(m_resource_map))
            wl_resource_set_implementation(resource->handle, nullptr, nullptr, nullptr);

        if (m_global) {
            wl_global_destroy(m_global);
            wl_list_remove(&m_displayDestroyedListener.link);
        }
    }

    void xdg_wm_base::init(struct ::wl_client *client, int id, int version)
    {
        m_resource = bind(client, id, version);
    }

    void xdg_wm_base::init(struct ::wl_resource *resource)
    {
        m_resource = bind(resource);
    }

    xdg_wm_base::Resource *xdg_wm_base::add(struct ::wl_client *client, int version)
    {
        Resource *resource = bind(client, 0, version);
        m_resource_map.insert(client, resource);
        return resource;
    }

    xdg_wm_base::Resource *xdg_wm_base::add(struct ::wl_client *client, int id, int version)
    {
        Resource *resource = bind(client, id, version);
        m_resource_map.insert(client, resource);
        return resource;
    }

    void xdg_wm_base::init(struct ::wl_display *display, int version)
    {
        m_global = wl_global_create(display, &::xdg_wm_base_interface, version, this, bind_func);
        m_globalVersion = version;
        m_displayDestroyedListener.notify = xdg_wm_base::display_destroy_func;
        m_displayDestroyedListener.parent = this;
        wl_display_add_destroy_listener(display, &m_displayDestroyedListener);
    }

    const struct wl_interface *xdg_wm_base::interface()
    {
        return &::xdg_wm_base_interface;
    }

    xdg_wm_base::Resource *xdg_wm_base::xdg_wm_base_allocate()
    {
        return new Resource;
    }

    void xdg_wm_base::xdg_wm_base_bind_resource(Resource *)
    {
    }

    void xdg_wm_base::xdg_wm_base_destroy_resource(Resource *)
    {
    }

    void xdg_wm_base::bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id)
    {
        xdg_wm_base *that = static_cast<xdg_wm_base *>(data);
        that->add(client, id, qMin(that->m_globalVersion, version));
    }

    void xdg_wm_base::display_destroy_func(struct ::wl_listener *listener, void *data)
    {
        Q_UNUSED(data);
        xdg_wm_base *that = static_cast<xdg_wm_base::DisplayDestroyedListener *>(listener)->parent;
        that->m_global = nullptr;
    }

    void xdg_wm_base::destroy_func(struct ::wl_resource *client_resource)
    {
        Resource *resource = Resource::fromResource(client_resource);
        xdg_wm_base *that = resource->xdg_wm_base_object;
        that->m_resource_map.remove(resource->client(), resource);
        that->xdg_wm_base_destroy_resource(resource);
        delete resource;
#if !WAYLAND_VERSION_CHECK(1, 2, 0)
        free(client_resource);
#endif
    }

    xdg_wm_base::Resource *xdg_wm_base::bind(struct ::wl_client *client, uint32_t id, int version)
    {
        Q_ASSERT_X(!wl_client_get_object(client, id), "QWaylandObject bind", QStringLiteral("binding to object %1 more than once").arg(id).toLocal8Bit().constData());
        struct ::wl_resource *handle = wl_resource_create(client, &::xdg_wm_base_interface, version, id);
        return bind(handle);
    }

    xdg_wm_base::Resource *xdg_wm_base::bind(struct ::wl_resource *handle)
    {
        Resource *resource = xdg_wm_base_allocate();
        resource->xdg_wm_base_object = this;

        wl_resource_set_implementation(handle, &m_xdg_wm_base_interface, resource, destroy_func);
        resource->handle = handle;
        xdg_wm_base_bind_resource(resource);
        return resource;
    }
    xdg_wm_base::Resource *xdg_wm_base::Resource::fromResource(struct ::wl_resource *resource)
    {
        if (wl_resource_instance_of(resource, &::xdg_wm_base_interface, &m_xdg_wm_base_interface))
            return static_cast<Resource *>(resource->data);
        return nullptr;
    }

    const struct ::xdg_wm_base_interface xdg_wm_base::m_xdg_wm_base_interface = {
        xdg_wm_base::handle_destroy,
        xdg_wm_base::handle_create_positioner,
        xdg_wm_base::handle_get_xdg_surface,
        xdg_wm_base::handle_pong
    };

    void xdg_wm_base::xdg_wm_base_destroy(Resource *)
    {
    }

    void xdg_wm_base::xdg_wm_base_create_positioner(Resource *, uint32_t)
    {
    }

    void xdg_wm_base::xdg_wm_base_get_xdg_surface(Resource *, uint32_t, struct ::wl_resource *)
    {
    }

    void xdg_wm_base::xdg_wm_base_pong(Resource *, uint32_t )
    {
    }


    void xdg_wm_base::handle_destroy(
        ::wl_client *client,
        struct wl_resource *resource)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_wm_base *>(r->xdg_wm_base_object)->xdg_wm_base_destroy(
            r);
    }

    void xdg_wm_base::handle_create_positioner(
        ::wl_client *client,
        struct wl_resource *resource,
        uint32_t id)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_wm_base *>(r->xdg_wm_base_object)->xdg_wm_base_create_positioner(
            r,
            id);
    }

    void xdg_wm_base::handle_get_xdg_surface(
        ::wl_client *client,
        struct wl_resource *resource,
        uint32_t id,
        struct ::wl_resource *surface)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_wm_base *>(r->xdg_wm_base_object)->xdg_wm_base_get_xdg_surface(
            r,
            id,
            surface);
    }

    void xdg_wm_base::handle_pong(
        ::wl_client *client,
        struct wl_resource *resource,
        uint32_t serial)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_wm_base *>(r->xdg_wm_base_object)->xdg_wm_base_pong(
            r,
            serial);
    }

    void xdg_wm_base::send_ping(uint32_t serial)
    {
        send_ping(
            m_resource->handle,
            serial);
    }

    void xdg_wm_base::send_ping(struct ::wl_resource *resource, uint32_t serial)
    {
        xdg_wm_base_send_ping(
            resource,
            serial);
    }


    xdg_positioner::xdg_positioner(struct ::wl_client *client, int id, int version)
        : m_resource_map()
        , m_resource(nullptr)
        , m_global(nullptr)
    {
        init(client, id, version);
    }

    xdg_positioner::xdg_positioner(struct ::wl_display *display, int version)
        : m_resource_map()
        , m_resource(nullptr)
        , m_global(nullptr)
    {
        init(display, version);
    }

    xdg_positioner::xdg_positioner(struct ::wl_resource *resource)
        : m_resource_map()
        , m_resource(nullptr)
        , m_global(nullptr)
    {
        init(resource);
    }

    xdg_positioner::xdg_positioner()
        : m_resource_map()
        , m_resource(nullptr)
        , m_global(nullptr)
    {
    }

    xdg_positioner::~xdg_positioner()
    {
        for (auto resource : qAsConst(m_resource_map))
            wl_resource_set_implementation(resource->handle, nullptr, nullptr, nullptr);

        if (m_global) {
            wl_global_destroy(m_global);
            wl_list_remove(&m_displayDestroyedListener.link);
        }
    }

    void xdg_positioner::init(struct ::wl_client *client, int id, int version)
    {
        m_resource = bind(client, id, version);
    }

    void xdg_positioner::init(struct ::wl_resource *resource)
    {
        m_resource = bind(resource);
    }

    xdg_positioner::Resource *xdg_positioner::add(struct ::wl_client *client, int version)
    {
        Resource *resource = bind(client, 0, version);
        m_resource_map.insert(client, resource);
        return resource;
    }

    xdg_positioner::Resource *xdg_positioner::add(struct ::wl_client *client, int id, int version)
    {
        Resource *resource = bind(client, id, version);
        m_resource_map.insert(client, resource);
        return resource;
    }

    void xdg_positioner::init(struct ::wl_display *display, int version)
    {
        m_global = wl_global_create(display, &::xdg_positioner_interface, version, this, bind_func);
        m_globalVersion = version;
        m_displayDestroyedListener.notify = xdg_positioner::display_destroy_func;
        m_displayDestroyedListener.parent = this;
        wl_display_add_destroy_listener(display, &m_displayDestroyedListener);
    }

    const struct wl_interface *xdg_positioner::interface()
    {
        return &::xdg_positioner_interface;
    }

    xdg_positioner::Resource *xdg_positioner::xdg_positioner_allocate()
    {
        return new Resource;
    }

    void xdg_positioner::xdg_positioner_bind_resource(Resource *)
    {
    }

    void xdg_positioner::xdg_positioner_destroy_resource(Resource *)
    {
    }

    void xdg_positioner::bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id)
    {
        xdg_positioner *that = static_cast<xdg_positioner *>(data);
        that->add(client, id, qMin(that->m_globalVersion, version));
    }

    void xdg_positioner::display_destroy_func(struct ::wl_listener *listener, void *data)
    {
        Q_UNUSED(data);
        xdg_positioner *that = static_cast<xdg_positioner::DisplayDestroyedListener *>(listener)->parent;
        that->m_global = nullptr;
    }

    void xdg_positioner::destroy_func(struct ::wl_resource *client_resource)
    {
        Resource *resource = Resource::fromResource(client_resource);
        xdg_positioner *that = resource->xdg_positioner_object;
        that->m_resource_map.remove(resource->client(), resource);
        that->xdg_positioner_destroy_resource(resource);
        delete resource;
#if !WAYLAND_VERSION_CHECK(1, 2, 0)
        free(client_resource);
#endif
    }

    xdg_positioner::Resource *xdg_positioner::bind(struct ::wl_client *client, uint32_t id, int version)
    {
        Q_ASSERT_X(!wl_client_get_object(client, id), "QWaylandObject bind", QStringLiteral("binding to object %1 more than once").arg(id).toLocal8Bit().constData());
        struct ::wl_resource *handle = wl_resource_create(client, &::xdg_positioner_interface, version, id);
        return bind(handle);
    }

    xdg_positioner::Resource *xdg_positioner::bind(struct ::wl_resource *handle)
    {
        Resource *resource = xdg_positioner_allocate();
        resource->xdg_positioner_object = this;

        wl_resource_set_implementation(handle, &m_xdg_positioner_interface, resource, destroy_func);
        resource->handle = handle;
        xdg_positioner_bind_resource(resource);
        return resource;
    }
    xdg_positioner::Resource *xdg_positioner::Resource::fromResource(struct ::wl_resource *resource)
    {
        if (wl_resource_instance_of(resource, &::xdg_positioner_interface, &m_xdg_positioner_interface))
            return static_cast<Resource *>(resource->data);
        return nullptr;
    }

    const struct ::xdg_positioner_interface xdg_positioner::m_xdg_positioner_interface = {
        xdg_positioner::handle_destroy,
        xdg_positioner::handle_set_size,
        xdg_positioner::handle_set_anchor_rect,
        xdg_positioner::handle_set_anchor,
        xdg_positioner::handle_set_gravity,
        xdg_positioner::handle_set_constraint_adjustment,
        xdg_positioner::handle_set_offset,
        xdg_positioner::handle_set_reactive,
        xdg_positioner::handle_set_parent_size,
        xdg_positioner::handle_set_parent_configure
    };

    void xdg_positioner::xdg_positioner_destroy(Resource *)
    {
    }

    void xdg_positioner::xdg_positioner_set_size(Resource *, int32_t , int32_t )
    {
    }

    void xdg_positioner::xdg_positioner_set_anchor_rect(Resource *, int32_t , int32_t , int32_t , int32_t )
    {
    }

    void xdg_positioner::xdg_positioner_set_anchor(Resource *, uint32_t )
    {
    }

    void xdg_positioner::xdg_positioner_set_gravity(Resource *, uint32_t )
    {
    }

    void xdg_positioner::xdg_positioner_set_constraint_adjustment(Resource *, uint32_t )
    {
    }

    void xdg_positioner::xdg_positioner_set_offset(Resource *, int32_t , int32_t )
    {
    }

    void xdg_positioner::xdg_positioner_set_reactive(Resource *)
    {
    }

    void xdg_positioner::xdg_positioner_set_parent_size(Resource *, int32_t , int32_t )
    {
    }

    void xdg_positioner::xdg_positioner_set_parent_configure(Resource *, uint32_t )
    {
    }


    void xdg_positioner::handle_destroy(
        ::wl_client *client,
        struct wl_resource *resource)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_positioner *>(r->xdg_positioner_object)->xdg_positioner_destroy(
            r);
    }

    void xdg_positioner::handle_set_size(
        ::wl_client *client,
        struct wl_resource *resource,
        int32_t width,
        int32_t height)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_positioner *>(r->xdg_positioner_object)->xdg_positioner_set_size(
            r,
            width,
            height);
    }

    void xdg_positioner::handle_set_anchor_rect(
        ::wl_client *client,
        struct wl_resource *resource,
        int32_t x,
        int32_t y,
        int32_t width,
        int32_t height)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_positioner *>(r->xdg_positioner_object)->xdg_positioner_set_anchor_rect(
            r,
            x,
            y,
            width,
            height);
    }

    void xdg_positioner::handle_set_anchor(
        ::wl_client *client,
        struct wl_resource *resource,
        uint32_t anchor)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_positioner *>(r->xdg_positioner_object)->xdg_positioner_set_anchor(
            r,
            anchor);
    }

    void xdg_positioner::handle_set_gravity(
        ::wl_client *client,
        struct wl_resource *resource,
        uint32_t gravity)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_positioner *>(r->xdg_positioner_object)->xdg_positioner_set_gravity(
            r,
            gravity);
    }

    void xdg_positioner::handle_set_constraint_adjustment(
        ::wl_client *client,
        struct wl_resource *resource,
        uint32_t constraint_adjustment)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_positioner *>(r->xdg_positioner_object)->xdg_positioner_set_constraint_adjustment(
            r,
            constraint_adjustment);
    }

    void xdg_positioner::handle_set_offset(
        ::wl_client *client,
        struct wl_resource *resource,
        int32_t x,
        int32_t y)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_positioner *>(r->xdg_positioner_object)->xdg_positioner_set_offset(
            r,
            x,
            y);
    }

    void xdg_positioner::handle_set_reactive(
        ::wl_client *client,
        struct wl_resource *resource)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_positioner *>(r->xdg_positioner_object)->xdg_positioner_set_reactive(
            r);
    }

    void xdg_positioner::handle_set_parent_size(
        ::wl_client *client,
        struct wl_resource *resource,
        int32_t parent_width,
        int32_t parent_height)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_positioner *>(r->xdg_positioner_object)->xdg_positioner_set_parent_size(
            r,
            parent_width,
            parent_height);
    }

    void xdg_positioner::handle_set_parent_configure(
        ::wl_client *client,
        struct wl_resource *resource,
        uint32_t serial)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_positioner *>(r->xdg_positioner_object)->xdg_positioner_set_parent_configure(
            r,
            serial);
    }

    xdg_surface::xdg_surface(struct ::wl_client *client, int id, int version)
        : m_resource_map()
        , m_resource(nullptr)
        , m_global(nullptr)
    {
        init(client, id, version);
    }

    xdg_surface::xdg_surface(struct ::wl_display *display, int version)
        : m_resource_map()
        , m_resource(nullptr)
        , m_global(nullptr)
    {
        init(display, version);
    }

    xdg_surface::xdg_surface(struct ::wl_resource *resource)
        : m_resource_map()
        , m_resource(nullptr)
        , m_global(nullptr)
    {
        init(resource);
    }

    xdg_surface::xdg_surface()
        : m_resource_map()
        , m_resource(nullptr)
        , m_global(nullptr)
    {
    }

    xdg_surface::~xdg_surface()
    {
        for (auto resource : qAsConst(m_resource_map))
            wl_resource_set_implementation(resource->handle, nullptr, nullptr, nullptr);

        if (m_global) {
            wl_global_destroy(m_global);
            wl_list_remove(&m_displayDestroyedListener.link);
        }
    }

    void xdg_surface::init(struct ::wl_client *client, int id, int version)
    {
        m_resource = bind(client, id, version);
    }

    void xdg_surface::init(struct ::wl_resource *resource)
    {
        m_resource = bind(resource);
    }

    xdg_surface::Resource *xdg_surface::add(struct ::wl_client *client, int version)
    {
        Resource *resource = bind(client, 0, version);
        m_resource_map.insert(client, resource);
        return resource;
    }

    xdg_surface::Resource *xdg_surface::add(struct ::wl_client *client, int id, int version)
    {
        Resource *resource = bind(client, id, version);
        m_resource_map.insert(client, resource);
        return resource;
    }

    void xdg_surface::init(struct ::wl_display *display, int version)
    {
        m_global = wl_global_create(display, &::xdg_surface_interface, version, this, bind_func);
        m_globalVersion = version;
        m_displayDestroyedListener.notify = xdg_surface::display_destroy_func;
        m_displayDestroyedListener.parent = this;
        wl_display_add_destroy_listener(display, &m_displayDestroyedListener);
    }

    const struct wl_interface *xdg_surface::interface()
    {
        return &::xdg_surface_interface;
    }

    xdg_surface::Resource *xdg_surface::xdg_surface_allocate()
    {
        return new Resource;
    }

    void xdg_surface::xdg_surface_bind_resource(Resource *)
    {
    }

    void xdg_surface::xdg_surface_destroy_resource(Resource *)
    {
    }

    void xdg_surface::bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id)
    {
        xdg_surface *that = static_cast<xdg_surface *>(data);
        that->add(client, id, qMin(that->m_globalVersion, version));
    }

    void xdg_surface::display_destroy_func(struct ::wl_listener *listener, void *data)
    {
        Q_UNUSED(data);
        xdg_surface *that = static_cast<xdg_surface::DisplayDestroyedListener *>(listener)->parent;
        that->m_global = nullptr;
    }

    void xdg_surface::destroy_func(struct ::wl_resource *client_resource)
    {
        Resource *resource = Resource::fromResource(client_resource);
        xdg_surface *that = resource->xdg_surface_object;
        that->m_resource_map.remove(resource->client(), resource);
        that->xdg_surface_destroy_resource(resource);
        delete resource;
#if !WAYLAND_VERSION_CHECK(1, 2, 0)
        free(client_resource);
#endif
    }

    xdg_surface::Resource *xdg_surface::bind(struct ::wl_client *client, uint32_t id, int version)
    {
        Q_ASSERT_X(!wl_client_get_object(client, id), "QWaylandObject bind", QStringLiteral("binding to object %1 more than once").arg(id).toLocal8Bit().constData());
        struct ::wl_resource *handle = wl_resource_create(client, &::xdg_surface_interface, version, id);
        return bind(handle);
    }

    xdg_surface::Resource *xdg_surface::bind(struct ::wl_resource *handle)
    {
        Resource *resource = xdg_surface_allocate();
        resource->xdg_surface_object = this;

        wl_resource_set_implementation(handle, &m_xdg_surface_interface, resource, destroy_func);
        resource->handle = handle;
        xdg_surface_bind_resource(resource);
        return resource;
    }
    xdg_surface::Resource *xdg_surface::Resource::fromResource(struct ::wl_resource *resource)
    {
        if (wl_resource_instance_of(resource, &::xdg_surface_interface, &m_xdg_surface_interface))
            return static_cast<Resource *>(resource->data);
        return nullptr;
    }

    const struct ::xdg_surface_interface xdg_surface::m_xdg_surface_interface = {
        xdg_surface::handle_destroy,
        xdg_surface::handle_get_toplevel,
        xdg_surface::handle_get_popup,
        xdg_surface::handle_set_window_geometry,
        xdg_surface::handle_ack_configure
    };

    void xdg_surface::xdg_surface_destroy(Resource *)
    {
    }

    void xdg_surface::xdg_surface_get_toplevel(Resource *, uint32_t)
    {
    }

    void xdg_surface::xdg_surface_get_popup(Resource *, uint32_t, struct ::wl_resource *, struct ::wl_resource *)
    {
    }

    void xdg_surface::xdg_surface_set_window_geometry(Resource *, int32_t , int32_t , int32_t , int32_t )
    {
    }

    void xdg_surface::xdg_surface_ack_configure(Resource *, uint32_t )
    {
    }


    void xdg_surface::handle_destroy(
        ::wl_client *client,
        struct wl_resource *resource)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_surface *>(r->xdg_surface_object)->xdg_surface_destroy(
            r);
    }

    void xdg_surface::handle_get_toplevel(
        ::wl_client *client,
        struct wl_resource *resource,
        uint32_t id)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_surface *>(r->xdg_surface_object)->xdg_surface_get_toplevel(
            r,
            id);
    }

    void xdg_surface::handle_get_popup(
        ::wl_client *client,
        struct wl_resource *resource,
        uint32_t id,
        struct ::wl_resource *parent,
        struct ::wl_resource *positioner)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_surface *>(r->xdg_surface_object)->xdg_surface_get_popup(
            r,
            id,
            parent,
            positioner);
    }

    void xdg_surface::handle_set_window_geometry(
        ::wl_client *client,
        struct wl_resource *resource,
        int32_t x,
        int32_t y,
        int32_t width,
        int32_t height)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_surface *>(r->xdg_surface_object)->xdg_surface_set_window_geometry(
            r,
            x,
            y,
            width,
            height);
    }

    void xdg_surface::handle_ack_configure(
        ::wl_client *client,
        struct wl_resource *resource,
        uint32_t serial)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_surface *>(r->xdg_surface_object)->xdg_surface_ack_configure(
            r,
            serial);
    }

    void xdg_surface::send_configure(uint32_t serial)
    {
        send_configure(
            m_resource->handle,
            serial);
    }

    void xdg_surface::send_configure(struct ::wl_resource *resource, uint32_t serial)
    {
        xdg_surface_send_configure(
            resource,
            serial);
    }


    xdg_toplevel::xdg_toplevel(struct ::wl_client *client, int id, int version)
        : m_resource_map()
        , m_resource(nullptr)
        , m_global(nullptr)
    {
        init(client, id, version);
    }

    xdg_toplevel::xdg_toplevel(struct ::wl_display *display, int version)
        : m_resource_map()
        , m_resource(nullptr)
        , m_global(nullptr)
    {
        init(display, version);
    }

    xdg_toplevel::xdg_toplevel(struct ::wl_resource *resource)
        : m_resource_map()
        , m_resource(nullptr)
        , m_global(nullptr)
    {
        init(resource);
    }

    xdg_toplevel::xdg_toplevel()
        : m_resource_map()
        , m_resource(nullptr)
        , m_global(nullptr)
    {
    }

    xdg_toplevel::~xdg_toplevel()
    {
        for (auto resource : qAsConst(m_resource_map))
            wl_resource_set_implementation(resource->handle, nullptr, nullptr, nullptr);

        if (m_global) {
            wl_global_destroy(m_global);
            wl_list_remove(&m_displayDestroyedListener.link);
        }
    }

    void xdg_toplevel::init(struct ::wl_client *client, int id, int version)
    {
        m_resource = bind(client, id, version);
    }

    void xdg_toplevel::init(struct ::wl_resource *resource)
    {
        m_resource = bind(resource);
    }

    xdg_toplevel::Resource *xdg_toplevel::add(struct ::wl_client *client, int version)
    {
        Resource *resource = bind(client, 0, version);
        m_resource_map.insert(client, resource);
        return resource;
    }

    xdg_toplevel::Resource *xdg_toplevel::add(struct ::wl_client *client, int id, int version)
    {
        Resource *resource = bind(client, id, version);
        m_resource_map.insert(client, resource);
        return resource;
    }

    void xdg_toplevel::init(struct ::wl_display *display, int version)
    {
        m_global = wl_global_create(display, &::xdg_toplevel_interface, version, this, bind_func);
        m_globalVersion = version;
        m_displayDestroyedListener.notify = xdg_toplevel::display_destroy_func;
        m_displayDestroyedListener.parent = this;
        wl_display_add_destroy_listener(display, &m_displayDestroyedListener);
    }

    const struct wl_interface *xdg_toplevel::interface()
    {
        return &::xdg_toplevel_interface;
    }

    xdg_toplevel::Resource *xdg_toplevel::xdg_toplevel_allocate()
    {
        return new Resource;
    }

    void xdg_toplevel::xdg_toplevel_bind_resource(Resource *)
    {
    }

    void xdg_toplevel::xdg_toplevel_destroy_resource(Resource *)
    {
    }

    void xdg_toplevel::bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id)
    {
        xdg_toplevel *that = static_cast<xdg_toplevel *>(data);
        that->add(client, id, qMin(that->m_globalVersion, version));
    }

    void xdg_toplevel::display_destroy_func(struct ::wl_listener *listener, void *data)
    {
        Q_UNUSED(data);
        xdg_toplevel *that = static_cast<xdg_toplevel::DisplayDestroyedListener *>(listener)->parent;
        that->m_global = nullptr;
    }

    void xdg_toplevel::destroy_func(struct ::wl_resource *client_resource)
    {
        Resource *resource = Resource::fromResource(client_resource);
        xdg_toplevel *that = resource->xdg_toplevel_object;
        that->m_resource_map.remove(resource->client(), resource);
        that->xdg_toplevel_destroy_resource(resource);
        delete resource;
#if !WAYLAND_VERSION_CHECK(1, 2, 0)
        free(client_resource);
#endif
    }

    xdg_toplevel::Resource *xdg_toplevel::bind(struct ::wl_client *client, uint32_t id, int version)
    {
        Q_ASSERT_X(!wl_client_get_object(client, id), "QWaylandObject bind", QStringLiteral("binding to object %1 more than once").arg(id).toLocal8Bit().constData());
        struct ::wl_resource *handle = wl_resource_create(client, &::xdg_toplevel_interface, version, id);
        return bind(handle);
    }

    xdg_toplevel::Resource *xdg_toplevel::bind(struct ::wl_resource *handle)
    {
        Resource *resource = xdg_toplevel_allocate();
        resource->xdg_toplevel_object = this;

        wl_resource_set_implementation(handle, &m_xdg_toplevel_interface, resource, destroy_func);
        resource->handle = handle;
        xdg_toplevel_bind_resource(resource);
        return resource;
    }
    xdg_toplevel::Resource *xdg_toplevel::Resource::fromResource(struct ::wl_resource *resource)
    {
        if (wl_resource_instance_of(resource, &::xdg_toplevel_interface, &m_xdg_toplevel_interface))
            return static_cast<Resource *>(resource->data);
        return nullptr;
    }

    const struct ::xdg_toplevel_interface xdg_toplevel::m_xdg_toplevel_interface = {
        xdg_toplevel::handle_destroy,
        xdg_toplevel::handle_set_parent,
        xdg_toplevel::handle_set_title,
        xdg_toplevel::handle_set_app_id,
        xdg_toplevel::handle_show_window_menu,
        xdg_toplevel::handle_move,
        xdg_toplevel::handle_resize,
        xdg_toplevel::handle_set_max_size,
        xdg_toplevel::handle_set_min_size,
        xdg_toplevel::handle_set_maximized,
        xdg_toplevel::handle_unset_maximized,
        xdg_toplevel::handle_set_fullscreen,
        xdg_toplevel::handle_unset_fullscreen,
        xdg_toplevel::handle_set_minimized
    };

    void xdg_toplevel::xdg_toplevel_destroy(Resource *)
    {
    }

    void xdg_toplevel::xdg_toplevel_set_parent(Resource *, struct ::wl_resource *)
    {
    }

    void xdg_toplevel::xdg_toplevel_set_title(Resource *, const QString &)
    {
    }

    void xdg_toplevel::xdg_toplevel_set_app_id(Resource *, const QString &)
    {
    }

    void xdg_toplevel::xdg_toplevel_show_window_menu(Resource *, struct ::wl_resource *, uint32_t , int32_t , int32_t )
    {
    }

    void xdg_toplevel::xdg_toplevel_move(Resource *, struct ::wl_resource *, uint32_t )
    {
    }

    void xdg_toplevel::xdg_toplevel_resize(Resource *, struct ::wl_resource *, uint32_t , uint32_t )
    {
    }

    void xdg_toplevel::xdg_toplevel_set_max_size(Resource *, int32_t , int32_t )
    {
    }

    void xdg_toplevel::xdg_toplevel_set_min_size(Resource *, int32_t , int32_t )
    {
    }

    void xdg_toplevel::xdg_toplevel_set_maximized(Resource *)
    {
    }

    void xdg_toplevel::xdg_toplevel_unset_maximized(Resource *)
    {
    }

    void xdg_toplevel::xdg_toplevel_set_fullscreen(Resource *, struct ::wl_resource *)
    {
    }

    void xdg_toplevel::xdg_toplevel_unset_fullscreen(Resource *)
    {
    }

    void xdg_toplevel::xdg_toplevel_set_minimized(Resource *)
    {
    }


    void xdg_toplevel::handle_destroy(
        ::wl_client *client,
        struct wl_resource *resource)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_toplevel *>(r->xdg_toplevel_object)->xdg_toplevel_destroy(
            r);
    }

    void xdg_toplevel::handle_set_parent(
        ::wl_client *client,
        struct wl_resource *resource,
        struct ::wl_resource *parent)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_toplevel *>(r->xdg_toplevel_object)->xdg_toplevel_set_parent(
            r,
            parent);
    }

    void xdg_toplevel::handle_set_title(
        ::wl_client *client,
        struct wl_resource *resource,
        const char *title)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_toplevel *>(r->xdg_toplevel_object)->xdg_toplevel_set_title(
            r,
            QString::fromUtf8(title));
    }

    void xdg_toplevel::handle_set_app_id(
        ::wl_client *client,
        struct wl_resource *resource,
        const char *app_id)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_toplevel *>(r->xdg_toplevel_object)->xdg_toplevel_set_app_id(
            r,
            QString::fromUtf8(app_id));
    }

    void xdg_toplevel::handle_show_window_menu(
        ::wl_client *client,
        struct wl_resource *resource,
        struct ::wl_resource *seat,
        uint32_t serial,
        int32_t x,
        int32_t y)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_toplevel *>(r->xdg_toplevel_object)->xdg_toplevel_show_window_menu(
            r,
            seat,
            serial,
            x,
            y);
    }

    void xdg_toplevel::handle_move(
        ::wl_client *client,
        struct wl_resource *resource,
        struct ::wl_resource *seat,
        uint32_t serial)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_toplevel *>(r->xdg_toplevel_object)->xdg_toplevel_move(
            r,
            seat,
            serial);
    }

    void xdg_toplevel::handle_resize(
        ::wl_client *client,
        struct wl_resource *resource,
        struct ::wl_resource *seat,
        uint32_t serial,
        uint32_t edges)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_toplevel *>(r->xdg_toplevel_object)->xdg_toplevel_resize(
            r,
            seat,
            serial,
            edges);
    }

    void xdg_toplevel::handle_set_max_size(
        ::wl_client *client,
        struct wl_resource *resource,
        int32_t width,
        int32_t height)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_toplevel *>(r->xdg_toplevel_object)->xdg_toplevel_set_max_size(
            r,
            width,
            height);
    }

    void xdg_toplevel::handle_set_min_size(
        ::wl_client *client,
        struct wl_resource *resource,
        int32_t width,
        int32_t height)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_toplevel *>(r->xdg_toplevel_object)->xdg_toplevel_set_min_size(
            r,
            width,
            height);
    }

    void xdg_toplevel::handle_set_maximized(
        ::wl_client *client,
        struct wl_resource *resource)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_toplevel *>(r->xdg_toplevel_object)->xdg_toplevel_set_maximized(
            r);
    }

    void xdg_toplevel::handle_unset_maximized(
        ::wl_client *client,
        struct wl_resource *resource)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_toplevel *>(r->xdg_toplevel_object)->xdg_toplevel_unset_maximized(
            r);
    }

    void xdg_toplevel::handle_set_fullscreen(
        ::wl_client *client,
        struct wl_resource *resource,
        struct ::wl_resource *output)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_toplevel *>(r->xdg_toplevel_object)->xdg_toplevel_set_fullscreen(
            r,
            output);
    }

    void xdg_toplevel::handle_unset_fullscreen(
        ::wl_client *client,
        struct wl_resource *resource)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_toplevel *>(r->xdg_toplevel_object)->xdg_toplevel_unset_fullscreen(
            r);
    }

    void xdg_toplevel::handle_set_minimized(
        ::wl_client *client,
        struct wl_resource *resource)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_toplevel *>(r->xdg_toplevel_object)->xdg_toplevel_set_minimized(
            r);
    }

    void xdg_toplevel::send_configure(int32_t width, int32_t height, const QByteArray &states)
    {
        send_configure(
            m_resource->handle,
            width,
            height,
            states);
    }

    void xdg_toplevel::send_configure(struct ::wl_resource *resource, int32_t width, int32_t height, const QByteArray &states)
    {
        struct wl_array states_data;
        states_data.size = states.size();
        states_data.data = static_cast<void *>(const_cast<char *>(states.constData()));
        states_data.alloc = 0;

        xdg_toplevel_send_configure(
            resource,
            width,
            height,
            &states_data);
    }


    void xdg_toplevel::send_close()
    {
        send_close(
            m_resource->handle);
    }

    void xdg_toplevel::send_close(struct ::wl_resource *resource)
    {
        xdg_toplevel_send_close(
            resource);
    }


    void xdg_toplevel::send_configure_bounds(int32_t width, int32_t height)
    {
        send_configure_bounds(
            m_resource->handle,
            width,
            height);
    }

    void xdg_toplevel::send_configure_bounds(struct ::wl_resource *resource, int32_t width, int32_t height)
    {
        xdg_toplevel_send_configure_bounds(
            resource,
            width,
            height);
    }


    void xdg_toplevel::send_wm_capabilities(const QByteArray &capabilities)
    {
        send_wm_capabilities(
            m_resource->handle,
            capabilities);
    }

    void xdg_toplevel::send_wm_capabilities(struct ::wl_resource *resource, const QByteArray &capabilities)
    {
        struct wl_array capabilities_data;
        capabilities_data.size = capabilities.size();
        capabilities_data.data = static_cast<void *>(const_cast<char *>(capabilities.constData()));
        capabilities_data.alloc = 0;

        xdg_toplevel_send_wm_capabilities(
            resource,
            &capabilities_data);
    }


    xdg_popup::xdg_popup(struct ::wl_client *client, int id, int version)
        : m_resource_map()
        , m_resource(nullptr)
        , m_global(nullptr)
    {
        init(client, id, version);
    }

    xdg_popup::xdg_popup(struct ::wl_display *display, int version)
        : m_resource_map()
        , m_resource(nullptr)
        , m_global(nullptr)
    {
        init(display, version);
    }

    xdg_popup::xdg_popup(struct ::wl_resource *resource)
        : m_resource_map()
        , m_resource(nullptr)
        , m_global(nullptr)
    {
        init(resource);
    }

    xdg_popup::xdg_popup()
        : m_resource_map()
        , m_resource(nullptr)
        , m_global(nullptr)
    {
    }

    xdg_popup::~xdg_popup()
    {
        for (auto resource : qAsConst(m_resource_map))
            wl_resource_set_implementation(resource->handle, nullptr, nullptr, nullptr);

        if (m_global) {
            wl_global_destroy(m_global);
            wl_list_remove(&m_displayDestroyedListener.link);
        }
    }

    void xdg_popup::init(struct ::wl_client *client, int id, int version)
    {
        m_resource = bind(client, id, version);
    }

    void xdg_popup::init(struct ::wl_resource *resource)
    {
        m_resource = bind(resource);
    }

    xdg_popup::Resource *xdg_popup::add(struct ::wl_client *client, int version)
    {
        Resource *resource = bind(client, 0, version);
        m_resource_map.insert(client, resource);
        return resource;
    }

    xdg_popup::Resource *xdg_popup::add(struct ::wl_client *client, int id, int version)
    {
        Resource *resource = bind(client, id, version);
        m_resource_map.insert(client, resource);
        return resource;
    }

    void xdg_popup::init(struct ::wl_display *display, int version)
    {
        m_global = wl_global_create(display, &::xdg_popup_interface, version, this, bind_func);
        m_globalVersion = version;
        m_displayDestroyedListener.notify = xdg_popup::display_destroy_func;
        m_displayDestroyedListener.parent = this;
        wl_display_add_destroy_listener(display, &m_displayDestroyedListener);
    }

    const struct wl_interface *xdg_popup::interface()
    {
        return &::xdg_popup_interface;
    }

    xdg_popup::Resource *xdg_popup::xdg_popup_allocate()
    {
        return new Resource;
    }

    void xdg_popup::xdg_popup_bind_resource(Resource *)
    {
    }

    void xdg_popup::xdg_popup_destroy_resource(Resource *)
    {
    }

    void xdg_popup::bind_func(struct ::wl_client *client, void *data, uint32_t version, uint32_t id)
    {
        xdg_popup *that = static_cast<xdg_popup *>(data);
        that->add(client, id, qMin(that->m_globalVersion, version));
    }

    void xdg_popup::display_destroy_func(struct ::wl_listener *listener, void *data)
    {
        Q_UNUSED(data);
        xdg_popup *that = static_cast<xdg_popup::DisplayDestroyedListener *>(listener)->parent;
        that->m_global = nullptr;
    }

    void xdg_popup::destroy_func(struct ::wl_resource *client_resource)
    {
        Resource *resource = Resource::fromResource(client_resource);
        xdg_popup *that = resource->xdg_popup_object;
        that->m_resource_map.remove(resource->client(), resource);
        that->xdg_popup_destroy_resource(resource);
        delete resource;
#if !WAYLAND_VERSION_CHECK(1, 2, 0)
        free(client_resource);
#endif
    }

    xdg_popup::Resource *xdg_popup::bind(struct ::wl_client *client, uint32_t id, int version)
    {
        Q_ASSERT_X(!wl_client_get_object(client, id), "QWaylandObject bind", QStringLiteral("binding to object %1 more than once").arg(id).toLocal8Bit().constData());
        struct ::wl_resource *handle = wl_resource_create(client, &::xdg_popup_interface, version, id);
        return bind(handle);
    }

    xdg_popup::Resource *xdg_popup::bind(struct ::wl_resource *handle)
    {
        Resource *resource = xdg_popup_allocate();
        resource->xdg_popup_object = this;

        wl_resource_set_implementation(handle, &m_xdg_popup_interface, resource, destroy_func);
        resource->handle = handle;
        xdg_popup_bind_resource(resource);
        return resource;
    }
    xdg_popup::Resource *xdg_popup::Resource::fromResource(struct ::wl_resource *resource)
    {
        if (wl_resource_instance_of(resource, &::xdg_popup_interface, &m_xdg_popup_interface))
            return static_cast<Resource *>(resource->data);
        return nullptr;
    }

    const struct ::xdg_popup_interface xdg_popup::m_xdg_popup_interface = {
        xdg_popup::handle_destroy,
        xdg_popup::handle_grab,
        xdg_popup::handle_reposition
    };

    void xdg_popup::xdg_popup_destroy(Resource *)
    {
    }

    void xdg_popup::xdg_popup_grab(Resource *, struct ::wl_resource *, uint32_t )
    {
    }

    void xdg_popup::xdg_popup_reposition(Resource *, struct ::wl_resource *, uint32_t )
    {
    }


    void xdg_popup::handle_destroy(
        ::wl_client *client,
        struct wl_resource *resource)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_popup *>(r->xdg_popup_object)->xdg_popup_destroy(
            r);
    }

    void xdg_popup::handle_grab(
        ::wl_client *client,
        struct wl_resource *resource,
        struct ::wl_resource *seat,
        uint32_t serial)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_popup *>(r->xdg_popup_object)->xdg_popup_grab(
            r,
            seat,
            serial);
    }

    void xdg_popup::handle_reposition(
        ::wl_client *client,
        struct wl_resource *resource,
        struct ::wl_resource *positioner,
        uint32_t token)
    {
        Q_UNUSED(client);
        Resource *r = Resource::fromResource(resource);
        static_cast<xdg_popup *>(r->xdg_popup_object)->xdg_popup_reposition(
            r,
            positioner,
            token);
    }

    void xdg_popup::send_configure(int32_t x, int32_t y, int32_t width, int32_t height)
    {
        send_configure(
            m_resource->handle,
            x,
            y,
            width,
            height);
    }

    void xdg_popup::send_configure(struct ::wl_resource *resource, int32_t x, int32_t y, int32_t width, int32_t height)
    {
        xdg_popup_send_configure(
            resource,
            x,
            y,
            width,
            height);
    }


    void xdg_popup::send_popup_done()
    {
        send_popup_done(
            m_resource->handle);
    }

    void xdg_popup::send_popup_done(struct ::wl_resource *resource)
    {
        xdg_popup_send_popup_done(
            resource);
    }


    void xdg_popup::send_repositioned(uint32_t token)
    {
        send_repositioned(
            m_resource->handle,
            token);
    }

    void xdg_popup::send_repositioned(struct ::wl_resource *resource, uint32_t token)
    {
        xdg_popup_send_repositioned(
            resource,
            token);
    }

}

QT_WARNING_POP
QT_END_NAMESPACE
