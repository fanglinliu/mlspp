#include "credential.h"

namespace mls {

///
/// CredentialType
///

tls::ostream&
operator<<(tls::ostream& out, CredentialType type)
{
  return out << uint8_t(type);
}

tls::istream&
operator>>(tls::istream& in, CredentialType& type)
{
  uint8_t temp;
  in >> temp;
  type = CredentialType(temp);
  return in;
}

///
/// BasicCredential
///

std::unique_ptr<AbstractCredential>
BasicCredential::dup() const
{
  return std::make_unique<BasicCredential>(_identity, _public_key);
}

bytes
BasicCredential::identity() const
{
  return _identity;
}

SignaturePublicKey
BasicCredential::public_key() const
{
  return _public_key;
}

void
BasicCredential::read(tls::istream& in)
{
  SignatureScheme scheme;
  in >> _identity >> scheme;

  _public_key = SignaturePublicKey(scheme);
  in >> _public_key;
}

void
BasicCredential::write(tls::ostream& out) const
{
  out << _identity << _public_key.signature_scheme() << _public_key;
}

bool
BasicCredential::equal(const AbstractCredential* other) const
{
  auto basic_other = dynamic_cast<const BasicCredential*>(other);
  return (_identity == basic_other->_identity) &&
         (_public_key == basic_other->_public_key);
}

///
/// Credential
///

bytes
Credential::identity() const
{
  return _cred->identity();
}

SignaturePublicKey
Credential::public_key() const
{
  return _cred->public_key();
}

bool
Credential::valid_for(const SignaturePrivateKey& priv) const
{
  return priv.public_key() == public_key();
}

Credential
Credential::basic(const bytes& identity, const SignaturePublicKey& public_key)
{
  auto cred = Credential{};
  cred._type = CredentialType::basic;
  cred._cred = std::make_unique<BasicCredential>(identity, public_key);
  return cred;
}

Credential
Credential::basic(const bytes& identity, const SignaturePrivateKey& private_key)
{
  // XXX(rlb@ipv.sx): This might merit invesetigation, but for now,
  // just disabling the check.  It seems like this is just a
  // pass-through, so how could it leak?
  return basic(identity, private_key.public_key());
}

AbstractCredential*
Credential::create(CredentialType type)
{
  switch (type) {
    case CredentialType::basic:
      return new BasicCredential();

    case CredentialType::x509:
      throw NotImplementedError();

    default:
      throw InvalidParameterError("Unknown credential type");
  }
}

bool
operator==(const Credential& lhs, const Credential& rhs)
{
  auto type = (lhs._type == rhs._type);
  auto cred = lhs._cred->equal(rhs._cred.get());
  return type && cred;
}

bool
operator!=(const Credential& lhs, const Credential& rhs)
{
  return !(lhs == rhs);
}

tls::ostream&
operator<<(tls::ostream& out, const Credential& obj)
{
  out << obj._type;
  obj._cred->write(out);
  return out;
}

tls::istream&
operator>>(tls::istream& in, Credential& obj)
{
  in >> obj._type;
  obj._cred.reset(Credential::create(obj._type));
  obj._cred->read(in);
  return in;
}

} // namespace mls