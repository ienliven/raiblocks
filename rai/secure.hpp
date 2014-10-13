#include <boost/iostreams/stream.hpp>
#include <boost/filesystem.hpp>
#include <boost/multiprecision/cpp_int.hpp>

#include <cryptopp/sha3.h>

#include <leveldb/db.h>

#include <ed25519-donna/ed25519.h>

#include <unordered_map>

namespace rai
{
	using stream = std::basic_streambuf <uint8_t>;
	template <typename T>
	bool read (rai::stream & stream_a, T & value)
	{
		auto amount_read (stream_a.sgetn (reinterpret_cast <uint8_t *> (&value), sizeof (value)));
		return amount_read != sizeof (value);
	}
	template <typename T>
	void write (rai::stream & stream_a, T const & value)
	{
		auto amount_written (stream_a.sputn (reinterpret_cast <uint8_t const *> (&value), sizeof (value)));
		assert (amount_written == sizeof (value));
	}
	using uint128_t = boost::multiprecision::uint128_t;
	using uint256_t = boost::multiprecision::uint256_t;
	using uint512_t = boost::multiprecision::uint512_t;
	union uint128_union
	{
	public:
		uint128_union () = default;
		uint128_union (rai::uint128_union const &) = default;
		uint128_union (rai::uint128_t const &);
		std::array <uint8_t, 16> bytes;
		std::array <uint64_t, 2> qwords;
	};
	union uint256_union
	{
		uint256_union () = default;
		uint256_union (std::string const &);
		uint256_union (uint64_t);
		uint256_union (rai::uint256_t const &);
		uint256_union (rai::uint256_union const &, rai::uint256_union const &, uint128_union const &);
		void digest_password (std::string const &);
		uint256_union prv (uint256_union const &, uint128_union const &) const;
		uint256_union & operator = (leveldb::Slice const &);
		uint256_union & operator ^= (rai::uint256_union const &);
		uint256_union operator ^ (rai::uint256_union const &) const;
		bool operator == (rai::uint256_union const &) const;
		bool operator != (rai::uint256_union const &) const;
		bool operator < (rai::uint256_union const &) const;
		void encode_hex (std::string &) const;
		bool decode_hex (std::string const &);
		void encode_dec (std::string &) const;
		bool decode_dec (std::string const &);
		void encode_base58check (std::string &) const;
		bool decode_base58check (std::string const &);
		void serialize (rai::stream &) const;
		bool deserialize (rai::stream &);
		std::array <uint8_t, 32> bytes;
		std::array <char, 32> chars;
		std::array <uint64_t, 4> qwords;
		std::array <uint128_union, 2> owords;
		void clear ();
		bool is_zero () const;
		std::string to_string () const;
		rai::uint256_t number () const;
	};
	using block_hash = uint256_union;
	using identifier = uint256_union;
	using address = uint256_union;
	using balance = uint256_union;
	using amount = uint256_union;
	using public_key = uint256_union;
	using private_key = uint256_union;
	using secret_key = uint256_union;
	using checksum = uint256_union;
	union uint512_union
	{
		uint512_union () = default;
		uint512_union (rai::uint512_t const &);
		bool operator == (rai::uint512_union const &) const;
		bool operator != (rai::uint512_union const &) const;
		rai::uint512_union & operator ^= (rai::uint512_union const &);
		void encode_hex (std::string &);
		bool decode_hex (std::string const &);
		rai::uint512_union salsa20_8 ();
		std::array <uint8_t, 64> bytes;
		std::array <uint32_t, 16> dwords;
		std::array <uint64_t, 8> qwords;
		std::array <uint256_union, 2> uint256s;
		void clear ();
		boost::multiprecision::uint512_t number ();
	};
	using signature = uint512_union;
	class keypair
	{
	public:
		keypair ();
		keypair (std::string const &);
		rai::public_key pub;
		rai::private_key prv;
	};
}
namespace std
{
	template <>
	struct hash <rai::uint256_union>
	{
		size_t operator () (rai::uint256_union const & data_a) const
		{
			return *reinterpret_cast <size_t const *> (data_a.bytes.data ());
		}
	};
	template <>
	struct hash <rai::uint256_t>
	{
		size_t operator () (rai::uint256_t const & number_a) const
		{
			return number_a.convert_to <size_t> ();
		}
	};
}
namespace boost
{
	template <>
	struct hash <rai::uint256_union>
	{
		size_t operator () (rai::uint256_union const & value_a) const
		{
			std::hash <rai::uint256_union> hash;
			return hash (value_a);
		}
	};
}
namespace rai
{
	void sign_message (rai::private_key const &, rai::public_key const &, rai::uint256_union const &, rai::uint512_union &);
	bool validate_message (rai::public_key const &, rai::uint256_union const &, rai::uint512_union const &);
	class block_visitor;
	enum class block_type : uint8_t
	{
		invalid,
		not_a_block,
		send,
		receive,
		open,
		change
	};
	class block
	{
	public:
		rai::uint256_union hash () const;
		virtual void hash (CryptoPP::SHA3 &) const = 0;
		virtual rai::block_hash previous () const = 0;
		virtual rai::block_hash source () const = 0;
		virtual void serialize (rai::stream &) const = 0;
		virtual void visit (rai::block_visitor &) const = 0;
		virtual bool operator == (rai::block const &) const = 0;
		virtual std::unique_ptr <rai::block> clone () const = 0;
		virtual rai::block_type type () const = 0;
	};
	std::unique_ptr <rai::block> deserialize_block (rai::stream &);
	void serialize_block (rai::stream &, rai::block const &);
	class send_hashables
	{
	public:
		void hash (CryptoPP::SHA3 &) const;
		rai::address destination;
		rai::block_hash previous;
		rai::uint256_union balance;
	};
	class send_block : public rai::block
	{
	public:
		send_block () = default;
		send_block (send_block const &);
		using rai::block::hash;
		void hash (CryptoPP::SHA3 &) const override;
		rai::block_hash previous () const override;
		rai::block_hash source () const override;
		void serialize (rai::stream &) const override;
		bool deserialize (rai::stream &);
		void visit (rai::block_visitor &) const override;
		std::unique_ptr <rai::block> clone () const override;
		rai::block_type type () const override;
		bool operator == (rai::block const &) const override;
		bool operator == (rai::send_block const &) const;
		send_hashables hashables;
		rai::signature signature;
	};
	class receive_hashables
	{
	public:
		void hash (CryptoPP::SHA3 &) const;
		rai::block_hash previous;
		rai::block_hash source;
	};
	class receive_block : public rai::block
	{
	public:
		using rai::block::hash;
		void hash (CryptoPP::SHA3 &) const override;
		rai::block_hash previous () const override;
		rai::block_hash source () const override;
		void serialize (rai::stream &) const override;
		bool deserialize (rai::stream &);
		void visit (rai::block_visitor &) const override;
		std::unique_ptr <rai::block> clone () const override;
		rai::block_type type () const override;
		void sign (rai::private_key const &, rai::public_key const &, rai::uint256_union const &);
		bool validate (rai::public_key const &, rai::uint256_t const &) const;
		bool operator == (rai::block const &) const override;
		bool operator == (rai::receive_block const &) const;
		receive_hashables hashables;
		uint512_union signature;
	};
	class open_hashables
	{
	public:
		void hash (CryptoPP::SHA3 &) const;
		rai::address representative;
		rai::block_hash source;
	};
	class open_block : public rai::block
	{
	public:
		using rai::block::hash;
		void hash (CryptoPP::SHA3 &) const override;
		rai::block_hash previous () const override;
		rai::block_hash source () const override;
		void serialize (rai::stream &) const override;
		bool deserialize (rai::stream &);
		void visit (rai::block_visitor &) const override;
		std::unique_ptr <rai::block> clone () const override;
		rai::block_type type () const override;
		bool operator == (rai::block const &) const override;
		bool operator == (rai::open_block const &) const;
		rai::open_hashables hashables;
		rai::uint512_union signature;
	};
	class change_hashables
	{
	public:
		void hash (CryptoPP::SHA3 &) const;
		rai::address representative;
		rai::block_hash previous;
	};
	class change_block : public rai::block
	{
	public:
		using rai::block::hash;
		void hash (CryptoPP::SHA3 &) const override;
		rai::block_hash previous () const override;
		rai::block_hash source () const override;
		void serialize (rai::stream &) const override;
		bool deserialize (rai::stream &);
		void visit (rai::block_visitor &) const override;
		std::unique_ptr <rai::block> clone () const override;
		rai::block_type type () const override;
		bool operator == (rai::block const &) const override;
		bool operator == (rai::change_block const &) const;
		rai::change_hashables hashables;
		rai::uint512_union signature;
	};
	class block_visitor
	{
	public:
		virtual void send_block (rai::send_block const &) = 0;
		virtual void receive_block (rai::receive_block const &) = 0;
		virtual void open_block (rai::open_block const &) = 0;
		virtual void change_block (rai::change_block const &) = 0;
	};
	struct block_store_temp_t
	{
	};
	class frontier
	{
	public:
		void serialize (rai::stream &) const;
		bool deserialize (rai::stream &);
		bool operator == (rai::frontier const &) const;
		rai::uint256_union hash;
		rai::address representative;
		rai::uint256_union balance;
		uint64_t time;
	};
	class account_entry
	{
	public:
		account_entry * operator -> ();
		rai::address first;
		rai::frontier second;
	};
	class account_iterator
	{
	public:
		account_iterator (leveldb::DB &);
		account_iterator (leveldb::DB &, std::nullptr_t);
		account_iterator (leveldb::DB &, rai::address const &);
		account_iterator (rai::account_iterator &&) = default;
		account_iterator & operator ++ ();
		account_iterator & operator = (rai::account_iterator &&) = default;
		account_entry & operator -> ();
		bool operator == (rai::account_iterator const &) const;
		bool operator != (rai::account_iterator const &) const;
		void set_current ();
		std::unique_ptr <leveldb::Iterator> iterator;
		rai::account_entry current;
	};
	class block_entry
	{
	public:
		block_entry * operator -> ();
		rai::block_hash first;
		std::unique_ptr <rai::block> second;
	};
	class block_iterator
	{
	public:
		block_iterator (leveldb::DB &);
		block_iterator (leveldb::DB &, std::nullptr_t);
		block_iterator (rai::block_iterator &&) = default;
		block_iterator & operator ++ ();
		block_entry & operator -> ();
		bool operator == (rai::block_iterator const &) const;
		bool operator != (rai::block_iterator const &) const;
		void set_current ();
		std::unique_ptr <leveldb::Iterator> iterator;
		rai::block_entry current;
	};
	extern block_store_temp_t block_store_temp;
	class block_store
	{
	public:
		block_store (block_store_temp_t const &);
		block_store (boost::filesystem::path const &);
		
		uint64_t now ();
		
		rai::block_hash root (rai::block const &);
		void block_put (rai::block_hash const &, rai::block const &);
		std::unique_ptr <rai::block> block_get (rai::block_hash const &);
		void block_del (rai::block_hash const &);
		bool block_exists (rai::block_hash const &);
		block_iterator blocks_begin ();
		block_iterator blocks_end ();
		
		void latest_put (rai::address const &, rai::frontier const &);
		bool latest_get (rai::address const &, rai::frontier &);
		void latest_del (rai::address const &);
		bool latest_exists (rai::address const &);
		account_iterator latest_begin (rai::address const &);
		account_iterator latest_begin ();
		account_iterator latest_end ();
		
		void pending_put (rai::block_hash const &, rai::address const &, rai::uint256_union const &, rai::address const &);
		void pending_del (rai::identifier const &);
		bool pending_get (rai::identifier const &, rai::address &, rai::uint256_union &, rai::address &);
		bool pending_exists (rai::block_hash const &);
		
		rai::uint256_t representation_get (rai::address const &);
		void representation_put (rai::address const &, rai::uint256_t const &);
		
		void fork_put (rai::block_hash const &, rai::block const &);
		std::unique_ptr <rai::block> fork_get (rai::block_hash const &);
		
		void bootstrap_put (rai::block_hash const &, rai::block const &);
		std::unique_ptr <rai::block> bootstrap_get (rai::block_hash const &);
		void bootstrap_del (rai::block_hash const &);
		
		void checksum_put (uint64_t, uint8_t, rai::checksum const &);
		bool checksum_get (uint64_t, uint8_t, rai::checksum &);
		void checksum_del (uint64_t, uint8_t);
		
	private:
		// address -> block_hash, representative, balance, timestamp    // Address to frontier block, representative, balance, last_change
		std::unique_ptr <leveldb::DB> addresses;
		// block_hash -> block                                          // Mapping block hash to contents
		std::unique_ptr <leveldb::DB> blocks;
		// block_hash -> sender, amount, destination                    // Pending blocks to sender address, amount, destination address
		std::unique_ptr <leveldb::DB> pending;
		// address -> weight                                            // Representation
		std::unique_ptr <leveldb::DB> representation;
		// block_hash -> sequence, block                                // Previous block hash to most recent sequence and fork proof
		std::unique_ptr <leveldb::DB> forks;
		// block_hash -> block                                          // Unchecked bootstrap blocks
		std::unique_ptr <leveldb::DB> bootstrap;
		// block_hash -> block_hash                                     // Tracking successors for bootstrapping
		std::unique_ptr <leveldb::DB> successors;
		// (uint56_t, uint8_t) -> block_hash                            // Mapping of region to checksum
		std::unique_ptr <leveldb::DB> checksum;
	};
	enum class process_result
	{
		progress, // Hasn't been seen before, signed correctly
		bad_signature, // One or more signatures was bad, forged or transmission error
		old, // Already seen and was valid
		overspend, // Malicious attempt to overspend
		overreceive, // Malicious attempt to receive twice
		fork, // Malicious fork of existing block
		gap_previous, // Block marked as previous isn't in store
		gap_source, // Block marked as source isn't in store
		not_receive_from_send // Receive does not have a send source
	};
	class ledger
	{
	public:
		ledger (rai::block_store &);
		rai::address account (rai::block_hash const &);
		rai::uint256_t amount (rai::block_hash const &);
		rai::uint256_t balance (rai::block_hash const &);
		rai::uint256_t account_balance (rai::address const &);
		rai::uint256_t weight (rai::address const &);
		std::unique_ptr <rai::block> successor (rai::block_hash const &);
		rai::block_hash latest (rai::address const &);
		rai::address representative (rai::block_hash const &);
		rai::address representative_calculated (rai::block_hash const &);
		rai::address representative_cached (rai::block_hash const &);
		rai::uint256_t supply ();
		rai::process_result process (rai::block const &);
		void rollback (rai::block_hash const &);
		void change_latest (rai::address const &, rai::block_hash const &, rai::address const &, rai::uint256_union const &);
		void move_representation (rai::address const &, rai::address const &, rai::uint256_t const &);
		void checksum_update (rai::block_hash const &);
		rai::checksum checksum (rai::address const &, rai::address const &);
		rai::block_store & store;
	};
	class vote
	{
	public:
		rai::uint256_union hash () const;
		rai::address address;
		rai::signature signature;
		uint64_t sequence;
		std::unique_ptr <rai::block> block;
	};
	class votes
	{
	public:
		votes (rai::ledger &, rai::block const &);
		void vote (rai::vote const &);
		std::pair <std::unique_ptr <rai::block>, rai::uint256_t> winner ();
		rai::uint256_t flip_threshold ();
		rai::ledger & ledger;
		rai::block_hash const root;
		std::unique_ptr <rai::block> last_winner;
		uint64_t sequence;
		std::unordered_map <rai::address, std::pair <uint64_t, std::unique_ptr <rai::block>>> rep_votes;
	};
}