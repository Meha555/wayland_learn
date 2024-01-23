#include "qwayland-xdg-shell.h"

QT_BEGIN_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wmissing-field-initializers")

namespace QtWayland {
    xdg_wm_base::xdg_wm_base(struct ::wl_registry *registry, int id, int version)
    {
        init(registry, id, version);
    }

    xdg_wm_base::xdg_wm_base(struct ::xdg_wm_base *obj)
        : m_xdg_wm_base(obj)
    {
        init_listener();
    }

    xdg_wm_base::xdg_wm_base()
        : m_xdg_wm_base(nullptr)
    {
    }

    xdg_wm_base::~xdg_wm_base()
    {
    }

    void xdg_wm_base::init(struct ::wl_registry *registry, int id, int version)
    {
        m_xdg_wm_base = static_cast<struct ::xdg_wm_base *>(wl_registry_bind(registry, id, &xdg_wm_base_interface, version));
        init_listener();
    }

    void xdg_wm_base::init(struct ::xdg_wm_base *obj)
    {
        m_xdg_wm_base = obj;
        init_listener();
    }

    bool xdg_wm_base::isInitialized() const
    {
        return m_xdg_wm_base != nullptr;
    }

    const struct wl_interface *xdg_wm_base::interface()
    {
        return &::xdg_wm_base_interface;
    }

    void xdg_wm_base::destroy()
    {
        xdg_wm_base_destroy(
            m_xdg_wm_base);
        m_xdg_wm_base = nullptr;
    }

    struct ::xdg_positioner *xdg_wm_base::create_positioner()
    {
        return xdg_wm_base_create_positioner(
            m_xdg_wm_base);
    }

    struct ::xdg_surface *xdg_wm_base::get_xdg_surface(struct ::wl_surface *surface)
    {
        return xdg_wm_base_get_xdg_surface(
            m_xdg_wm_base,
            surface);
    }

    void xdg_wm_base::pong(uint32_t serial)
    {
        xdg_wm_base_pong(
            m_xdg_wm_base,
            serial);
    }

    void xdg_wm_base::xdg_wm_base_ping(uint32_t )
    {
    }

    void xdg_wm_base::handle_ping(
        void *data,
        struct ::xdg_wm_base *object,
        uint32_t serial)
    {
        Q_UNUSED(object);
        static_cast<xdg_wm_base *>(data)->xdg_wm_base_ping(
            serial);
    }

    const struct xdg_wm_base_listener xdg_wm_base::m_xdg_wm_base_listener = {
        xdg_wm_base::handle_ping
    };

    void xdg_wm_base::init_listener()
    {
        xdg_wm_base_add_listener(m_xdg_wm_base, &m_xdg_wm_base_listener, this);
    }

    xdg_positioner::xdg_positioner(struct ::wl_registry *registry, int id, int version)
    {
        init(registry, id, version);
    }

    xdg_positioner::xdg_positioner(struct ::xdg_positioner *obj)
        : m_xdg_positioner(obj)
    {
    }

    xdg_positioner::xdg_positioner()
        : m_xdg_positioner(nullptr)
    {
    }

    xdg_positioner::~xdg_positioner()
    {
    }

    void xdg_positioner::init(struct ::wl_registry *registry, int id, int version)
    {
        m_xdg_positioner = static_cast<struct ::xdg_positioner *>(wl_registry_bind(registry, id, &xdg_positioner_interface, version));
    }

    void xdg_positioner::init(struct ::xdg_positioner *obj)
    {
        m_xdg_positioner = obj;
    }

    bool xdg_positioner::isInitialized() const
    {
        return m_xdg_positioner != nullptr;
    }

    const struct wl_interface *xdg_positioner::interface()
    {
        return &::xdg_positioner_interface;
    }

    void xdg_positioner::destroy()
    {
        xdg_positioner_destroy(
            m_xdg_positioner);
        m_xdg_positioner = nullptr;
    }

    void xdg_positioner::set_size(int32_t width, int32_t height)
    {
        xdg_positioner_set_size(
            m_xdg_positioner,
            width,
            height);
    }

    void xdg_positioner::set_anchor_rect(int32_t x, int32_t y, int32_t width, int32_t height)
    {
        xdg_positioner_set_anchor_rect(
            m_xdg_positioner,
            x,
            y,
            width,
            height);
    }

    void xdg_positioner::set_anchor(uint32_t anchor)
    {
        xdg_positioner_set_anchor(
            m_xdg_positioner,
            anchor);
    }

    void xdg_positioner::set_gravity(uint32_t gravity)
    {
        xdg_positioner_set_gravity(
            m_xdg_positioner,
            gravity);
    }

    void xdg_positioner::set_constraint_adjustment(uint32_t constraint_adjustment)
    {
        xdg_positioner_set_constraint_adjustment(
            m_xdg_positioner,
            constraint_adjustment);
    }

    void xdg_positioner::set_offset(int32_t x, int32_t y)
    {
        xdg_positioner_set_offset(
            m_xdg_positioner,
            x,
            y);
    }

    void xdg_positioner::set_reactive()
    {
        xdg_positioner_set_reactive(
            m_xdg_positioner);
    }

    void xdg_positioner::set_parent_size(int32_t parent_width, int32_t parent_height)
    {
        xdg_positioner_set_parent_size(
            m_xdg_positioner,
            parent_width,
            parent_height);
    }

    void xdg_positioner::set_parent_configure(uint32_t serial)
    {
        xdg_positioner_set_parent_configure(
            m_xdg_positioner,
            serial);
    }

    xdg_surface::xdg_surface(struct ::wl_registry *registry, int id, int version)
    {
        init(registry, id, version);
    }

    xdg_surface::xdg_surface(struct ::xdg_surface *obj)
        : m_xdg_surface(obj)
    {
        init_listener();
    }

    xdg_surface::xdg_surface()
        : m_xdg_surface(nullptr)
    {
    }

    xdg_surface::~xdg_surface()
    {
    }

    void xdg_surface::init(struct ::wl_registry *registry, int id, int version)
    {
        m_xdg_surface = static_cast<struct ::xdg_surface *>(wl_registry_bind(registry, id, &xdg_surface_interface, version));
        init_listener();
    }

    void xdg_surface::init(struct ::xdg_surface *obj)
    {
        m_xdg_surface = obj;
        init_listener();
    }

    bool xdg_surface::isInitialized() const
    {
        return m_xdg_surface != nullptr;
    }

    const struct wl_interface *xdg_surface::interface()
    {
        return &::xdg_surface_interface;
    }

    void xdg_surface::destroy()
    {
        xdg_surface_destroy(
            m_xdg_surface);
        m_xdg_surface = nullptr;
    }

    struct ::xdg_toplevel *xdg_surface::get_toplevel()
    {
        return xdg_surface_get_toplevel(
            m_xdg_surface);
    }

    struct ::xdg_popup *xdg_surface::get_popup(struct ::xdg_surface *parent, struct ::xdg_positioner *positioner)
    {
        return xdg_surface_get_popup(
            m_xdg_surface,
            parent,
            positioner);
    }

    void xdg_surface::set_window_geometry(int32_t x, int32_t y, int32_t width, int32_t height)
    {
        xdg_surface_set_window_geometry(
            m_xdg_surface,
            x,
            y,
            width,
            height);
    }

    void xdg_surface::ack_configure(uint32_t serial)
    {
        xdg_surface_ack_configure(
            m_xdg_surface,
            serial);
    }

    void xdg_surface::xdg_surface_configure(uint32_t )
    {
    }

    void xdg_surface::handle_configure(
        void *data,
        struct ::xdg_surface *object,
        uint32_t serial)
    {
        Q_UNUSED(object);
        static_cast<xdg_surface *>(data)->xdg_surface_configure(
            serial);
    }

    const struct xdg_surface_listener xdg_surface::m_xdg_surface_listener = {
        xdg_surface::handle_configure
    };

    void xdg_surface::init_listener()
    {
        xdg_surface_add_listener(m_xdg_surface, &m_xdg_surface_listener, this);
    }

    xdg_toplevel::xdg_toplevel(struct ::wl_registry *registry, int id, int version)
    {
        init(registry, id, version);
    }

    xdg_toplevel::xdg_toplevel(struct ::xdg_toplevel *obj)
        : m_xdg_toplevel(obj)
    {
        init_listener();
    }

    xdg_toplevel::xdg_toplevel()
        : m_xdg_toplevel(nullptr)
    {
    }

    xdg_toplevel::~xdg_toplevel()
    {
    }

    void xdg_toplevel::init(struct ::wl_registry *registry, int id, int version)
    {
        m_xdg_toplevel = static_cast<struct ::xdg_toplevel *>(wl_registry_bind(registry, id, &xdg_toplevel_interface, version));
        init_listener();
    }

    void xdg_toplevel::init(struct ::xdg_toplevel *obj)
    {
        m_xdg_toplevel = obj;
        init_listener();
    }

    bool xdg_toplevel::isInitialized() const
    {
        return m_xdg_toplevel != nullptr;
    }

    const struct wl_interface *xdg_toplevel::interface()
    {
        return &::xdg_toplevel_interface;
    }

    void xdg_toplevel::destroy()
    {
        xdg_toplevel_destroy(
            m_xdg_toplevel);
        m_xdg_toplevel = nullptr;
    }

    void xdg_toplevel::set_parent(struct ::xdg_toplevel *parent)
    {
        xdg_toplevel_set_parent(
            m_xdg_toplevel,
            parent);
    }

    void xdg_toplevel::set_title(const QString &title)
    {
        xdg_toplevel_set_title(
            m_xdg_toplevel,
            title.toUtf8().constData());
    }

    void xdg_toplevel::set_app_id(const QString &app_id)
    {
        xdg_toplevel_set_app_id(
            m_xdg_toplevel,
            app_id.toUtf8().constData());
    }

    void xdg_toplevel::show_window_menu(struct ::wl_seat *seat, uint32_t serial, int32_t x, int32_t y)
    {
        xdg_toplevel_show_window_menu(
            m_xdg_toplevel,
            seat,
            serial,
            x,
            y);
    }

    void xdg_toplevel::move(struct ::wl_seat *seat, uint32_t serial)
    {
        xdg_toplevel_move(
            m_xdg_toplevel,
            seat,
            serial);
    }

    void xdg_toplevel::resize(struct ::wl_seat *seat, uint32_t serial, uint32_t edges)
    {
        xdg_toplevel_resize(
            m_xdg_toplevel,
            seat,
            serial,
            edges);
    }

    void xdg_toplevel::set_max_size(int32_t width, int32_t height)
    {
        xdg_toplevel_set_max_size(
            m_xdg_toplevel,
            width,
            height);
    }

    void xdg_toplevel::set_min_size(int32_t width, int32_t height)
    {
        xdg_toplevel_set_min_size(
            m_xdg_toplevel,
            width,
            height);
    }

    void xdg_toplevel::set_maximized()
    {
        xdg_toplevel_set_maximized(
            m_xdg_toplevel);
    }

    void xdg_toplevel::unset_maximized()
    {
        xdg_toplevel_unset_maximized(
            m_xdg_toplevel);
    }

    void xdg_toplevel::set_fullscreen(struct ::wl_output *output)
    {
        xdg_toplevel_set_fullscreen(
            m_xdg_toplevel,
            output);
    }

    void xdg_toplevel::unset_fullscreen()
    {
        xdg_toplevel_unset_fullscreen(
            m_xdg_toplevel);
    }

    void xdg_toplevel::set_minimized()
    {
        xdg_toplevel_set_minimized(
            m_xdg_toplevel);
    }

    void xdg_toplevel::xdg_toplevel_configure(int32_t , int32_t , wl_array *)
    {
    }

    void xdg_toplevel::handle_configure(
        void *data,
        struct ::xdg_toplevel *object,
        int32_t width,
        int32_t height,
        wl_array *states)
    {
        Q_UNUSED(object);
        static_cast<xdg_toplevel *>(data)->xdg_toplevel_configure(
            width,
            height,
            states);
    }

    void xdg_toplevel::xdg_toplevel_close()
    {
    }

    void xdg_toplevel::handle_close(
        void *data,
        struct ::xdg_toplevel *object)
    {
        Q_UNUSED(object);
        static_cast<xdg_toplevel *>(data)->xdg_toplevel_close();
    }

    void xdg_toplevel::xdg_toplevel_configure_bounds(int32_t , int32_t )
    {
    }

    void xdg_toplevel::handle_configure_bounds(
        void *data,
        struct ::xdg_toplevel *object,
        int32_t width,
        int32_t height)
    {
        Q_UNUSED(object);
        static_cast<xdg_toplevel *>(data)->xdg_toplevel_configure_bounds(
            width,
            height);
    }

    void xdg_toplevel::xdg_toplevel_wm_capabilities(wl_array *)
    {
    }

    void xdg_toplevel::handle_wm_capabilities(
        void *data,
        struct ::xdg_toplevel *object,
        wl_array *capabilities)
    {
        Q_UNUSED(object);
        static_cast<xdg_toplevel *>(data)->xdg_toplevel_wm_capabilities(
            capabilities);
    }

    const struct xdg_toplevel_listener xdg_toplevel::m_xdg_toplevel_listener = {
        xdg_toplevel::handle_configure,
        xdg_toplevel::handle_close,
        xdg_toplevel::handle_configure_bounds,
        xdg_toplevel::handle_wm_capabilities
    };

    void xdg_toplevel::init_listener()
    {
        xdg_toplevel_add_listener(m_xdg_toplevel, &m_xdg_toplevel_listener, this);
    }

    xdg_popup::xdg_popup(struct ::wl_registry *registry, int id, int version)
    {
        init(registry, id, version);
    }

    xdg_popup::xdg_popup(struct ::xdg_popup *obj)
        : m_xdg_popup(obj)
    {
        init_listener();
    }

    xdg_popup::xdg_popup()
        : m_xdg_popup(nullptr)
    {
    }

    xdg_popup::~xdg_popup()
    {
    }

    void xdg_popup::init(struct ::wl_registry *registry, int id, int version)
    {
        m_xdg_popup = static_cast<struct ::xdg_popup *>(wl_registry_bind(registry, id, &xdg_popup_interface, version));
        init_listener();
    }

    void xdg_popup::init(struct ::xdg_popup *obj)
    {
        m_xdg_popup = obj;
        init_listener();
    }

    bool xdg_popup::isInitialized() const
    {
        return m_xdg_popup != nullptr;
    }

    const struct wl_interface *xdg_popup::interface()
    {
        return &::xdg_popup_interface;
    }

    void xdg_popup::destroy()
    {
        xdg_popup_destroy(
            m_xdg_popup);
        m_xdg_popup = nullptr;
    }

    void xdg_popup::grab(struct ::wl_seat *seat, uint32_t serial)
    {
        xdg_popup_grab(
            m_xdg_popup,
            seat,
            serial);
    }

    void xdg_popup::reposition(struct ::xdg_positioner *positioner, uint32_t token)
    {
        xdg_popup_reposition(
            m_xdg_popup,
            positioner,
            token);
    }

    void xdg_popup::xdg_popup_configure(int32_t , int32_t , int32_t , int32_t )
    {
    }

    void xdg_popup::handle_configure(
        void *data,
        struct ::xdg_popup *object,
        int32_t x,
        int32_t y,
        int32_t width,
        int32_t height)
    {
        Q_UNUSED(object);
        static_cast<xdg_popup *>(data)->xdg_popup_configure(
            x,
            y,
            width,
            height);
    }

    void xdg_popup::xdg_popup_popup_done()
    {
    }

    void xdg_popup::handle_popup_done(
        void *data,
        struct ::xdg_popup *object)
    {
        Q_UNUSED(object);
        static_cast<xdg_popup *>(data)->xdg_popup_popup_done();
    }

    void xdg_popup::xdg_popup_repositioned(uint32_t )
    {
    }

    void xdg_popup::handle_repositioned(
        void *data,
        struct ::xdg_popup *object,
        uint32_t token)
    {
        Q_UNUSED(object);
        static_cast<xdg_popup *>(data)->xdg_popup_repositioned(
            token);
    }

    const struct xdg_popup_listener xdg_popup::m_xdg_popup_listener = {
        xdg_popup::handle_configure,
        xdg_popup::handle_popup_done,
        xdg_popup::handle_repositioned
    };

    void xdg_popup::init_listener()
    {
        xdg_popup_add_listener(m_xdg_popup, &m_xdg_popup_listener, this);
    }
}

QT_WARNING_POP
QT_END_NAMESPACE
