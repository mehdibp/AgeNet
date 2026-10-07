//
// Generated file, do not edit! Created by opp_msgtool 6.3 from src/messages/VehicleStatusMessage.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wshadow"
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Wunused-parameter"
#  pragma clang diagnostic ignored "-Wc++98-compat"
#  pragma clang diagnostic ignored "-Wunreachable-code-break"
#  pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wsuggest-attribute=noreturn"
#  pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif

#include <iostream>
#include <sstream>
#include <memory>
#include <type_traits>
#include "VehicleStatusMessage_m.h"

namespace omnetpp {

// Template pack/unpack rules. They are declared *after* a1l type-specific pack functions for multiple reasons.
// They are in the omnetpp namespace, to allow them to be found by argument-dependent lookup via the cCommBuffer argument

// Packing/unpacking an std::vector
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::vector<T,A>& v)
{
    int n = v.size();
    doParsimPacking(buffer, n);
    for (int i = 0; i < n; i++)
        doParsimPacking(buffer, v[i]);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::vector<T,A>& v)
{
    int n;
    doParsimUnpacking(buffer, n);
    v.resize(n);
    for (int i = 0; i < n; i++)
        doParsimUnpacking(buffer, v[i]);
}

// Packing/unpacking an std::list
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::list<T,A>& l)
{
    doParsimPacking(buffer, (int)l.size());
    for (typename std::list<T,A>::const_iterator it = l.begin(); it != l.end(); ++it)
        doParsimPacking(buffer, (T&)*it);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::list<T,A>& l)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        l.push_back(T());
        doParsimUnpacking(buffer, l.back());
    }
}

// Packing/unpacking an std::set
template<typename T, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::set<T,Tr,A>& s)
{
    doParsimPacking(buffer, (int)s.size());
    for (typename std::set<T,Tr,A>::const_iterator it = s.begin(); it != s.end(); ++it)
        doParsimPacking(buffer, *it);
}

template<typename T, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::set<T,Tr,A>& s)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        T x;
        doParsimUnpacking(buffer, x);
        s.insert(x);
    }
}

// Packing/unpacking an std::map
template<typename K, typename V, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::map<K,V,Tr,A>& m)
{
    doParsimPacking(buffer, (int)m.size());
    for (typename std::map<K,V,Tr,A>::const_iterator it = m.begin(); it != m.end(); ++it) {
        doParsimPacking(buffer, it->first);
        doParsimPacking(buffer, it->second);
    }
}

template<typename K, typename V, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::map<K,V,Tr,A>& m)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        K k; V v;
        doParsimUnpacking(buffer, k);
        doParsimUnpacking(buffer, v);
        m[k] = v;
    }
}

// Default pack/unpack function for arrays
template<typename T>
void doParsimArrayPacking(omnetpp::cCommBuffer *b, const T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimPacking(b, t[i]);
}

template<typename T>
void doParsimArrayUnpacking(omnetpp::cCommBuffer *b, T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimUnpacking(b, t[i]);
}

// Default rule to prevent compiler from choosing base class' doParsimPacking() function
template<typename T>
void doParsimPacking(omnetpp::cCommBuffer *, const T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimPacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

template<typename T>
void doParsimUnpacking(omnetpp::cCommBuffer *, T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimUnpacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

}  // namespace omnetpp

namespace veins {

Register_Class(VehicleStatusMessage)

VehicleStatusMessage::VehicleStatusMessage(const char *name, short kind) : ::veins::BaseFrame1609_4(name, kind)
{
}

VehicleStatusMessage::VehicleStatusMessage(const VehicleStatusMessage& other) : ::veins::BaseFrame1609_4(other)
{
    copy(other);
}

VehicleStatusMessage::~VehicleStatusMessage()
{
}

VehicleStatusMessage& VehicleStatusMessage::operator=(const VehicleStatusMessage& other)
{
    if (this == &other) return *this;
    ::veins::BaseFrame1609_4::operator=(other);
    copy(other);
    return *this;
}

void VehicleStatusMessage::copy(const VehicleStatusMessage& other)
{
    this->senderId = other.senderId;
    this->serial = other.serial;
    this->vehicleId = other.vehicleId;
    this->posX = other.posX;
    this->posY = other.posY;
    this->speed = other.speed;
    this->creationTime = other.creationTime;
}

void VehicleStatusMessage::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::veins::BaseFrame1609_4::parsimPack(b);
    doParsimPacking(b,this->senderId);
    doParsimPacking(b,this->serial);
    doParsimPacking(b,this->vehicleId);
    doParsimPacking(b,this->posX);
    doParsimPacking(b,this->posY);
    doParsimPacking(b,this->speed);
    doParsimPacking(b,this->creationTime);
}

void VehicleStatusMessage::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::veins::BaseFrame1609_4::parsimUnpack(b);
    doParsimUnpacking(b,this->senderId);
    doParsimUnpacking(b,this->serial);
    doParsimUnpacking(b,this->vehicleId);
    doParsimUnpacking(b,this->posX);
    doParsimUnpacking(b,this->posY);
    doParsimUnpacking(b,this->speed);
    doParsimUnpacking(b,this->creationTime);
}

int VehicleStatusMessage::getSenderId() const
{
    return this->senderId;
}

void VehicleStatusMessage::setSenderId(int senderId)
{
    this->senderId = senderId;
}

int VehicleStatusMessage::getSerial() const
{
    return this->serial;
}

void VehicleStatusMessage::setSerial(int serial)
{
    this->serial = serial;
}

const char * VehicleStatusMessage::getVehicleId() const
{
    return this->vehicleId.c_str();
}

void VehicleStatusMessage::setVehicleId(const char * vehicleId)
{
    this->vehicleId = vehicleId;
}

double VehicleStatusMessage::getPosX() const
{
    return this->posX;
}

void VehicleStatusMessage::setPosX(double posX)
{
    this->posX = posX;
}

double VehicleStatusMessage::getPosY() const
{
    return this->posY;
}

void VehicleStatusMessage::setPosY(double posY)
{
    this->posY = posY;
}

double VehicleStatusMessage::getSpeed() const
{
    return this->speed;
}

void VehicleStatusMessage::setSpeed(double speed)
{
    this->speed = speed;
}

::omnetpp::simtime_t VehicleStatusMessage::getCreationTime() const
{
    return this->creationTime;
}

void VehicleStatusMessage::setCreationTime(::omnetpp::simtime_t creationTime)
{
    this->creationTime = creationTime;
}

class VehicleStatusMessageDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_senderId,
        FIELD_serial,
        FIELD_vehicleId,
        FIELD_posX,
        FIELD_posY,
        FIELD_speed,
        FIELD_creationTime,
    };
  public:
    VehicleStatusMessageDescriptor();
    virtual ~VehicleStatusMessageDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(VehicleStatusMessageDescriptor)

VehicleStatusMessageDescriptor::VehicleStatusMessageDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(veins::VehicleStatusMessage)), "veins::BaseFrame1609_4")
{
    propertyNames = nullptr;
}

VehicleStatusMessageDescriptor::~VehicleStatusMessageDescriptor()
{
    delete[] propertyNames;
}

bool VehicleStatusMessageDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<VehicleStatusMessage *>(obj)!=nullptr;
}

const char **VehicleStatusMessageDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *VehicleStatusMessageDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int VehicleStatusMessageDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 7+base->getFieldCount() : 7;
}

unsigned int VehicleStatusMessageDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_senderId
        FD_ISEDITABLE,    // FIELD_serial
        FD_ISEDITABLE,    // FIELD_vehicleId
        FD_ISEDITABLE,    // FIELD_posX
        FD_ISEDITABLE,    // FIELD_posY
        FD_ISEDITABLE,    // FIELD_speed
        FD_ISEDITABLE,    // FIELD_creationTime
    };
    return (field >= 0 && field < 7) ? fieldTypeFlags[field] : 0;
}

const char *VehicleStatusMessageDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "senderId",
        "serial",
        "vehicleId",
        "posX",
        "posY",
        "speed",
        "creationTime",
    };
    return (field >= 0 && field < 7) ? fieldNames[field] : nullptr;
}

int VehicleStatusMessageDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "senderId") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "serial") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "vehicleId") == 0) return baseIndex + 2;
    if (strcmp(fieldName, "posX") == 0) return baseIndex + 3;
    if (strcmp(fieldName, "posY") == 0) return baseIndex + 4;
    if (strcmp(fieldName, "speed") == 0) return baseIndex + 5;
    if (strcmp(fieldName, "creationTime") == 0) return baseIndex + 6;
    return base ? base->findField(fieldName) : -1;
}

const char *VehicleStatusMessageDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",    // FIELD_senderId
        "int",    // FIELD_serial
        "string",    // FIELD_vehicleId
        "double",    // FIELD_posX
        "double",    // FIELD_posY
        "double",    // FIELD_speed
        "omnetpp::simtime_t",    // FIELD_creationTime
    };
    return (field >= 0 && field < 7) ? fieldTypeStrings[field] : nullptr;
}

const char **VehicleStatusMessageDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *VehicleStatusMessageDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int VehicleStatusMessageDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    VehicleStatusMessage *pp = omnetpp::fromAnyPtr<VehicleStatusMessage>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void VehicleStatusMessageDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    VehicleStatusMessage *pp = omnetpp::fromAnyPtr<VehicleStatusMessage>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'VehicleStatusMessage'", field);
    }
}

const char *VehicleStatusMessageDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    VehicleStatusMessage *pp = omnetpp::fromAnyPtr<VehicleStatusMessage>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string VehicleStatusMessageDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    VehicleStatusMessage *pp = omnetpp::fromAnyPtr<VehicleStatusMessage>(object); (void)pp;
    switch (field) {
        case FIELD_senderId: return long2string(pp->getSenderId());
        case FIELD_serial: return long2string(pp->getSerial());
        case FIELD_vehicleId: return oppstring2string(pp->getVehicleId());
        case FIELD_posX: return double2string(pp->getPosX());
        case FIELD_posY: return double2string(pp->getPosY());
        case FIELD_speed: return double2string(pp->getSpeed());
        case FIELD_creationTime: return simtime2string(pp->getCreationTime());
        default: return "";
    }
}

void VehicleStatusMessageDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    VehicleStatusMessage *pp = omnetpp::fromAnyPtr<VehicleStatusMessage>(object); (void)pp;
    switch (field) {
        case FIELD_senderId: pp->setSenderId(string2long(value)); break;
        case FIELD_serial: pp->setSerial(string2long(value)); break;
        case FIELD_vehicleId: pp->setVehicleId((value)); break;
        case FIELD_posX: pp->setPosX(string2double(value)); break;
        case FIELD_posY: pp->setPosY(string2double(value)); break;
        case FIELD_speed: pp->setSpeed(string2double(value)); break;
        case FIELD_creationTime: pp->setCreationTime(string2simtime(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'VehicleStatusMessage'", field);
    }
}

omnetpp::cValue VehicleStatusMessageDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    VehicleStatusMessage *pp = omnetpp::fromAnyPtr<VehicleStatusMessage>(object); (void)pp;
    switch (field) {
        case FIELD_senderId: return pp->getSenderId();
        case FIELD_serial: return pp->getSerial();
        case FIELD_vehicleId: return pp->getVehicleId();
        case FIELD_posX: return pp->getPosX();
        case FIELD_posY: return pp->getPosY();
        case FIELD_speed: return pp->getSpeed();
        case FIELD_creationTime: return pp->getCreationTime().dbl();
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'VehicleStatusMessage' as cValue -- field index out of range?", field);
    }
}

void VehicleStatusMessageDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    VehicleStatusMessage *pp = omnetpp::fromAnyPtr<VehicleStatusMessage>(object); (void)pp;
    switch (field) {
        case FIELD_senderId: pp->setSenderId(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_serial: pp->setSerial(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_vehicleId: pp->setVehicleId(value.stringValue()); break;
        case FIELD_posX: pp->setPosX(value.doubleValue()); break;
        case FIELD_posY: pp->setPosY(value.doubleValue()); break;
        case FIELD_speed: pp->setSpeed(value.doubleValue()); break;
        case FIELD_creationTime: pp->setCreationTime(value.doubleValue()); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'VehicleStatusMessage'", field);
    }
}

const char *VehicleStatusMessageDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

omnetpp::any_ptr VehicleStatusMessageDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    VehicleStatusMessage *pp = omnetpp::fromAnyPtr<VehicleStatusMessage>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void VehicleStatusMessageDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    VehicleStatusMessage *pp = omnetpp::fromAnyPtr<VehicleStatusMessage>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'VehicleStatusMessage'", field);
    }
}

}  // namespace veins

namespace omnetpp {

}  // namespace omnetpp

