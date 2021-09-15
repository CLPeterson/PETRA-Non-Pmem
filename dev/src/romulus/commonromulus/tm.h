/* Definitions for different Persistent Transactional Memories */

#ifdef ROMULUS_PTM
#include "../romulus/Romulus.hpp"
#define TM_WRITE_TRANSACTION   romulus::Romulus::write_transaction
#define TM_READ_TRANSACTION    romulus::Romulus::read_transaction
#define TM_BEGIN_TRANSACTION() romulus::gRom.begin_transaction()
#define TM_END_TRANSACTION()   romulus::gRom.end_transaction()
#define TM_ALLOC               romulus::Romulus::alloc
#define TM_FREE                romulus::Romulus::free
#define TM_PMALLOC             romulus::Romulus::pmalloc
#define TM_PFREE               romulus::Romulus::pfree
#define TM_TYPE                romulus::persist
#define TM_NAME                romulus::Romulus::className
#define TM_CONSISTENCY_CHECK   romulus::Romulus::consistency_check
#define TM_INIT                romulus::Romulus::init
#endif

#ifdef ROMULUS_LOG_PTM
#include "../romuluslog/RomulusLog.hpp"
#define TM_WRITE_TRANSACTION   romuluslog::RomulusLog::write_transaction
#define TM_READ_TRANSACTION    romuluslog::RomulusLog::read_transaction
#define TM_BEGIN_TRANSACTION() romuluslog::gRomLog.begin_transaction()
#define TM_END_TRANSACTION()   romuluslog::gRomLog.end_transaction()
#define TM_ALLOC               romuluslog::RomulusLog::alloc
#define TM_FREE                romuluslog::RomulusLog::free
#define TM_PMALLOC             romuluslog::RomulusLog::pmalloc
#define TM_PFREE               romuluslog::RomulusLog::pfree
#define TM_TYPE                romuluslog::persist
#define TM_NAME                romuluslog::RomulusLog::className
#define TM_CONSISTENCY_CHECK   romuluslog::RomulusLog::consistency_check
#define TM_INIT                romuluslog::RomulusLog::init
#endif

#ifdef ROMULUS_LR_PTM
#include "../romuluslr/RomulusLR.hpp"
#define TM_WRITE_TRANSACTION   romuluslr::RomulusLR::write_transaction
#define TM_READ_TRANSACTION    romuluslr::RomulusLR::read_transaction
#define TM_BEGIN_TRANSACTION() romuluslr::gRomLR.begin_transaction()
#define TM_END_TRANSACTION()   romuluslr::gRomLR.end_transaction()
#define TM_ALLOC               romuluslr::RomulusLR::alloc
#define TM_FREE                romuluslr::RomulusLR::free
#define TM_PMALLOC             romuluslr::RomulusLR::pmalloc
#define TM_PFREE               romuluslr::RomulusLR::pfree
#define TM_TYPE                romuluslr::persist
#define TM_NAME                romuluslr::RomulusLR::className
#define TM_CONSISTENCY_CHECK   romuluslr::RomulusLR::consistency_check
#define TM_INIT                romuluslr::RomulusLR::init
#endif

#ifdef ROMULUS_NI_PTM
#include "../romulusni/RomulusNI.hpp"
#define TM_WRITE_TRANSACTION   romulusni::RomulusNI::write_transaction
#define TM_READ_TRANSACTION    romulusni::RomulusNI::read_transaction
#define TM_BEGIN_TRANSACTION() romulusni::gRomNI.begin_transaction()
#define TM_END_TRANSACTION()   romulusni::gRomNI.end_transaction()
#define TM_ALLOC               romulusni::RomulusNI::alloc
#define TM_FREE                romulusni::RomulusNI::free
#define TM_PMALLOC             romulusni::RomulusNI::pmalloc
#define TM_PFREE               romulusni::RomulusNI::pfree
#define TM_TYPE                romulusni::persist
#define TM_NAME                romulusni::RomulusNI::className
#define TM_CONSISTENCY_CHECK   romulusni::RomulusNI::consistency_check
#define TM_INIT                romulusni::RomulusNI::init
#endif

#ifdef ROMULUS_SINGLE_FENCE_PTM
#include "../romulussf/RomulusSF.hpp"
#define TM_WRITE_TRANSACTION   romulussf::RomulusSF::write_transaction
#define TM_READ_TRANSACTION    romulussf::RomulusSF::read_transaction
#define TM_BEGIN_TRANSACTION() romulussf::gRomSF.begin_transaction()
#define TM_END_TRANSACTION()   romulussf::gRomSF.end_transaction()
#define TM_ALLOC               romulussf::RomulusSF::alloc
#define TM_FREE                romulussf::RomulusSF::free
#define TM_PMALLOC             romulussf::RomulusSF::pmalloc
#define TM_PFREE               romulussf::RomulusSF::pfree
#define TM_TYPE                romulussf::persist
#define TM_NAME                romulussf::RomulusSF::className
#define TM_CONSISTENCY_CHECK   romulussf::RomulusSF::consistency_check
#endif

#ifdef ROMULUS_LOGPWB_PTM
#include "../romuluslogPWB/RomulusLogPWB.hpp"
#define TM_WRITE_TRANSACTION   romuluslogPWB::RomulusLogPWB::write_transaction
#define TM_READ_TRANSACTION    romuluslogPWB::RomulusLogPWB::read_transaction
#define TM_BEGIN_TRANSACTION() romuluslogPWB::gRomLogPWB.begin_transaction()
#define TM_END_TRANSACTION()   romuluslogPWB::gRomLogPWB.end_transaction()
#define TM_ALLOC               romuluslogPWB::RomulusLogPWB::alloc
#define TM_FREE                romuluslogPWB::RomulusLogPWB::free
#define TM_PMALLOC             romuluslogPWB::RomulusLogPWB::pmalloc
#define TM_PFREE               romuluslogPWB::RomulusLogPWB::pfree
#define TM_TYPE                romuluslogPWB::persist
#define TM_NAME                romuluslogPWB::RomulusLogPWB::className
#define TM_CONSISTENCY_CHECK   romuluslogPWB::RomulusLogPWB::consistency_check
#endif

#ifdef ROMULUS_2FLR_PTM
#include "../romulus2flr/Romulus2FLR.hpp"
#define TM_WRITE_TRANSACTION   romulus2flr::Romulus2FLR::write_transaction
#define TM_READ_TRANSACTION    romulus2flr::Romulus2FLR::read_transaction
#define TM_BEGIN_TRANSACTION() romulus2flr::gRom2FLR.begin_transaction()
#define TM_END_TRANSACTION()   romulus2flr::gRom2FLR.end_transaction()
#define TM_ALLOC               romulus2flr::Romulus2FLR::alloc
#define TM_FREE                romulus2flr::Romulus2FLR::free
#define TM_PMALLOC             romulus2flr::Romulus2FLR::pmalloc
#define TM_PFREE               romulus2flr::Romulus2FLR::pfree
#define TM_TYPE                romulus2flr::persist
#define TM_NAME                romulus2flr::Romulus2FLR::className
#define TM_CONSISTENCY_CHECK   romulus2flr::Romulus2FLR::consistency_check
#endif

#ifdef MNEMOSYNE_PTM
#include "../mnemosyne/Mnemosyne.hpp"
#define TM_WRITE_TRANSACTION   mnemosyne::Mnemosyne::write_transaction
#define TM_READ_TRANSACTION    mnemosyne::Mnemosyne::read_transaction
#define TM_BEGIN_TRANSACTION() PTx {
#define TM_END_TRANSACTION()   }
#define TM_ALLOC               mnemosyne::Mnemosyne::alloc
#define TM_FREE                mnemosyne::Mnemosyne::free
#define TM_PMALLOC             mnemosyne::Mnemosyne::pmalloc
#define TM_PFREE               mnemosyne::Mnemosyne::pfree
#define TM_TYPE                mnemosyne::persist
#define TM_NAME                mnemosyne::Mnemosyne::className
#define TM_CONSISTENCY_CHECK   mnemosyne::Mnemosyne::consistency_check
#define TM_INIT                mnemosyne::Mnemosyne::init
#endif

#ifdef PMDK_PTM
#include "../pmdk/PMDKTM.hpp"
#define TM_WRITE_TRANSACTION   pmdk::PMDKTM::write_transaction
#define TM_READ_TRANSACTION    pmdk::PMDKTM::read_transaction
#define TM_BEGIN_TRANSACTION() pmem::obj::transaction::manual(pmdk::gpop)  // this might not work
#define TM_END_TRANSACTION()   pmem::obj::transaction::commit()            // this might not work
#define TM_ALLOC               pmdk::PMDKTM::alloc
#define TM_FREE                pmdk::PMDKTM::free
#define TM_PMALLOC             pmdk::PMDKTM::pmalloc
#define TM_PFREE               pmdk::PMDKTM::pfree
#define TM_TYPE                pmdk::persist
#define TM_NAME                pmdk::PMDKTM::className
#define TM_CONSISTENCY_CHECK   pmdk::PMDKTM::consistency_check
#define TM_INIT                pmdk::PMDKTM::init
#endif

#ifdef CRWWP_STM
#include "../stms/CRWWPTM.hpp"
#define TM_WRITE_TRANSACTION   crwwptm::write_transaction
#define TM_READ_TRANSACTION    crwwptm::read_transaction
#define TM_BEGIN_TRANSACTION() crwwptm::begin_transaction()
#define TM_END_TRANSACTION()   crwwptm::end_transaction()
#define TM_ALLOC               crwwptm::alloc
#define TM_FREE                crwwptm::free
#define TM_PMALLOC             crwwptm::pmalloc
#define TM_PFREE               crwwptm::pfree
#define TM_TYPE                crwwptm::tmtype
#define TM_NAME                crwwptm::CRWWPTM::className
#define TM_CONSISTENCY_CHECK   crwwptm::CRWWPTM::consistency_check
#endif

#ifdef NOTHING_STM
#include "../NothingTM.hpp"
#define TM_WRITE_TRANSACTION   nothingtm::write_transaction
#define TM_READ_TRANSACTION    nothingtm::read_transaction
#define TM_BEGIN_TRANSACTION   nothingtm::begin_transaction
#define TM_END_TRANSACTION     nothingtm::end_transaction
#define TM_ALLOC               nothingtm::alloc
#define TM_FREE                nothingtm::free
#define TM_PMALLOC             nothingtm::pmalloc
#define TM_PFREE               nothingtm::pfree
#define TM_TYPE                nothingtm::tmtype
#define TM_NAME                nothingtm::NothingTM::className
#define TM_CONSISTENCY_CHECK   nothingtm::NothingTM::consistency_check
#endif
