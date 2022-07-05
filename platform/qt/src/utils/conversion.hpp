#pragma once

#include <QMapLibreGL/Types>

#include "geojson.hpp"

#include <mbgl/style/conversion/geojson.hpp>
#include <mbgl/style/conversion_impl.hpp>
#include <mbgl/util/optional.hpp>

#include <QColor>
#include <QVariant>

namespace mbgl {
namespace style {
namespace conversion {

std::string convertColor(const QColor &color);

template <>
class ConversionTraits<QVariant> {
public:
    static bool isUndefined(const QVariant& value) {
        return value.isNull() || !value.isValid();
    }

    static bool isArray(const QVariant& value) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        return QMetaType::canConvert(value.metaType(), QMetaType(QMetaType::QVariantList));
#else
        return value.canConvert(QVariant::List);
#endif
    }

    static std::size_t arrayLength(const QVariant& value) {
        return value.toList().size();
    }

    static QVariant arrayMember(const QVariant& value, std::size_t i) {
        return value.toList()[static_cast<int>(i)];
    }

    static bool isObject(const QVariant& value) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        return QMetaType::canConvert(value.metaType(), QMetaType(QMetaType::QVariantMap))
            || value.typeId() == QMetaType::QByteArray
#else
        return value.canConvert(QVariant::Map)
            || value.type() == QVariant::ByteArray
#endif
            || QString(value.typeName()) == QStringLiteral("QMapLibreGL::Feature")
            || value.userType() == qMetaTypeId<QVector<QMapLibreGL::Feature>>()
            || value.userType() == qMetaTypeId<QList<QMapLibreGL::Feature>>();
    }

    static optional<QVariant> objectMember(const QVariant& value, const char* key) {
        auto map = value.toMap();
        auto iter = map.constFind(key);

        if (iter != map.constEnd()) {
            return iter.value();
        } else {
            return {};
        }
    }

    template <class Fn>
    static optional<Error> eachMember(const QVariant& value, Fn&& fn) {
        auto map = value.toMap();
        auto iter = map.constBegin();

        while (iter != map.constEnd()) {
            optional<Error> result = fn(iter.key().toStdString(), QVariant(iter.value()));
            if (result) {
                return result;
            }

            ++iter;
        }

        return {};
    }

    static optional<bool> toBool(const QVariant& value) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        if (value.typeId() == QMetaType::Bool) {
#else
        if (value.type() == QVariant::Bool) {
#endif
            return value.toBool();
        } else {
            return {};
        }
    }

    static optional<float> toNumber(const QVariant& value) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        if (value.typeId() == QMetaType::Int || value.typeId() == QMetaType::Double) {
#else
        if (value.type() == QVariant::Int || value.type() == QVariant::Double) {
#endif
            return value.toFloat();
        } else {
            return {};
        }
    }

    static optional<double> toDouble(const QVariant& value) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        if (value.typeId() == QMetaType::Int || value.typeId() == QMetaType::Double) {
#else
        if (value.type() == QVariant::Int || value.type() == QVariant::Double) {
#endif
            return value.toDouble();
        } else {
            return {};
        }
    }

    static optional<std::string> toString(const QVariant& value) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        if (value.typeId() == QMetaType::QString) {
            return value.toString().toStdString();
        } else if (value.typeId() == QMetaType::QColor) {
            return convertColor(value.value<QColor>());
        } else {
            return {};
        }
#else
        if (value.type() == QVariant::String) {
            return value.toString().toStdString();
        } else if (value.type() == QVariant::Color) {
            return convertColor(value.value<QColor>());
        } else {
            return {};
        }
#endif
    }

    static optional<Value> toValue(const QVariant& value) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        if (value.typeId() == QMetaType::Bool) {
            return { value.toBool() };
        } else if (value.typeId() == QMetaType::QString) {
            return { value.toString().toStdString() };
        } else if (value.typeId() == QMetaType::QColor) {
            return { convertColor(value.value<QColor>()) };
        } else if (value.typeId() == QMetaType::Int) {
            return { int64_t(value.toInt()) };
        } else if (QMetaType::canConvert(value.metaType(), QMetaType(QMetaType::Double))) {
            return { value.toDouble() };
        } else {
            return {};
        }
#else
        if (value.type() == QVariant::Bool) {
            return { value.toBool() };
        } else if (value.type() == QVariant::String) {
            return { value.toString().toStdString() };
        } else if (value.type() == QVariant::Color) {
            return { convertColor(value.value<QColor>()) };
        } else if (value.type() == QVariant::Int) {
            return { int64_t(value.toInt()) };
        } else if (value.canConvert(QVariant::Double)) {
            return { value.toDouble() };
        } else {
            return {};
        }
#endif
    }

    static optional<GeoJSON> toGeoJSON(const QVariant& value, Error& error) {
        if (value.typeName() == QStringLiteral("QMapLibreGL::Feature")) {
            return GeoJSON { QMapLibreGL::GeoJSON::asFeature(value.value<QMapLibreGL::Feature>()) };
        } else if (value.userType() == qMetaTypeId<QVector<QMapLibreGL::Feature>>()) {
            return featureCollectionToGeoJSON(value.value<QVector<QMapLibreGL::Feature>>());
        } else if (value.userType() == qMetaTypeId<QList<QMapLibreGL::Feature>>()) {
            return featureCollectionToGeoJSON(value.value<QList<QMapLibreGL::Feature>>());
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        } else if (value.typeId() != QMetaType::QByteArray) {
#else
        } else if (value.type() != QVariant::ByteArray) {
#endif
            error = { "JSON data must be in QByteArray" };
            return {};
        }

        QByteArray data = value.toByteArray();
        return parseGeoJSON(std::string(data.constData(), data.size()), error);
    }

private:
    template<typename T>
    static GeoJSON featureCollectionToGeoJSON(const T &features) {
        mapbox::feature::feature_collection<double> collection;
        collection.reserve(static_cast<std::size_t>(features.size()));
        for (const auto &feature : features) {
            collection.push_back(QMapLibreGL::GeoJSON::asFeature(feature));
        }
        return GeoJSON { std::move(collection) };
    }
};

template <class T, class...Args>
optional<T> convert(const QVariant& value, Error& error, Args&&...args) {
    return convert<T>(Convertible(value), error, std::forward<Args>(args)...);
}

inline std::string convertColor(const QColor &color) {
    return QString::asprintf("rgba(%d,%d,%d,%lf)",
        color.red(), color.green(), color.blue(), color.alphaF()).toStdString();
}

} // namespace conversion
} // namespace style
} // namespace mbgl
