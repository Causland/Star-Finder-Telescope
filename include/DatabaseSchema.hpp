#ifndef STARDATABASESCHEMA_DATABASESCHEMA_H_H
#define STARDATABASESCHEMA_DATABASESCHEMA_H_H

#include <sqlpp11/table.h>
#include <sqlpp11/data_types.h>
#include <sqlpp11/char_sequence.h>

namespace StarDatabaseSchema
{
  namespace TargetBody_
  {
    struct BodyId
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "bodyId";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T bodyId;
            T& operator()() { return bodyId; }
            const T& operator()() const { return bodyId; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::require_insert>;
    };
    struct BodyName
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "bodyName";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T bodyName;
            T& operator()() { return bodyName; }
            const T& operator()() const { return bodyName; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::text, sqlpp::tag::require_insert>;
    };
  } // namespace TargetBody_

  struct TargetBody: sqlpp::table_t<TargetBody,
               TargetBody_::BodyId,
               TargetBody_::BodyName>
  {
    struct _alias_t
    {
      static constexpr const char _literal[] =  "TargetBody";
      using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
      template<typename T>
      struct _member_t
      {
        T TargetBody;
        T& operator()() { return TargetBody; }
        const T& operator()() const { return TargetBody; }
      };
    };
  };
  namespace Ephemeris_
  {
    struct BodyId
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "bodyId";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T bodyId;
            T& operator()() { return bodyId; }
            const T& operator()() const { return bodyId; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::require_insert>;
    };
    struct Time
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "time";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T time;
            T& operator()() { return time; }
            const T& operator()() const { return time; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::text, sqlpp::tag::require_insert>;
    };
    struct Azimuth
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "azimuth";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T azimuth;
            T& operator()() { return azimuth; }
            const T& operator()() const { return azimuth; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::floating_point, sqlpp::tag::require_insert>;
    };
    struct Elevation
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "elevation";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T elevation;
            T& operator()() { return elevation; }
            const T& operator()() const { return elevation; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::floating_point, sqlpp::tag::require_insert>;
    };
  } // namespace Ephemeris_

  struct Ephemeris: sqlpp::table_t<Ephemeris,
               Ephemeris_::BodyId,
               Ephemeris_::Time,
               Ephemeris_::Azimuth,
               Ephemeris_::Elevation>
  {
    struct _alias_t
    {
      static constexpr const char _literal[] =  "Ephemeris";
      using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
      template<typename T>
      struct _member_t
      {
        T Ephemeris;
        T& operator()() { return Ephemeris; }
        const T& operator()() const { return Ephemeris; }
      };
    };
  };
} // namespace StarDatabaseSchema
#endif
