#pragma once

namespace http
{
    enum class method
    {
        Unknown,
        GET,
        HEAD,
        POST,
        PUT,
        DELETE
    };

    enum class version
    {
        Unknown,
        http10,
        http11,
        http20
    };

    enum class ReqHeader
    {
        Accept,
        AcceptCharset,
        AcceptEncoding,
        CacheControl,
        Connection,
        Cookie,
        ContentLength,
        ContentType,
        Date,
        Origin,
    };

    enum class ResHeader
    {
        AccessControlAllowOrigin,
        CacheControl,
        Connection,
        ContentEncoding,
        ContentLength,
        ContentType,
        Date,
        Server,
        SetCookie,
        Status,
    };

    enum class StateCode
    {
        Unknown = 0,

        OK = 200,
        NoContent = 204,
        PartialContent = 206,

        MovedPermanently = 301,
        Found = 302,
        SeeOther = 303,
        NotModified = 304,
        TemporaryRedirect = 307,

        BadRequest = 400,
        Unauthorized = 401,
        Forbidden = 403,
        NotFound = 404,

        InternalServerError = 500,
        NotImplemented = 501,
        ServiceUnavailable = 503,
    };

} // namespace http