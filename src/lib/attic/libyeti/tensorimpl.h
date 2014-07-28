/*
 *@BEGIN LICENSE
 *
 * PSI4: an ab initio quantum chemistry software package
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *@END LICENSE
 */

#ifndef TENSORIMPL_H
#define TENSORIMPL_H

#include "tensor.h"

namespace yeti {

template <
    class ValidatePolicy,
    class BranchRenewPolicy,
    class MempoolRetrievePolicy,
    class MetaDataBranchRetrievePolicy,
    class TensorDataControllerRetrievePolicy,
    class DataBranchRetrievePolicy,
    class BranchFinalizePolicy,
    class BranchFlushPolicy,
    class BranchReleasePolicy,
    class MetaDataNodeRetrievePolicy,
    class DataNodeRetrievePolicy,
    class DataStorageFlushPolicy,
    class ObsoletePolicy,
    class SyncPolicy,
    class DataClearPolicy,
    class OutOfCorePrefetchPolicy,
    class InCorePrefetchPolicy
>
class TensorControllerTemplate :
    public TensorController,
    /** Policies */
    public ValidatePolicy,
    public BranchRenewPolicy,
    public MempoolRetrievePolicy,
    public MetaDataBranchRetrievePolicy,
    public TensorDataControllerRetrievePolicy,
    public DataBranchRetrievePolicy,
    public BranchFlushPolicy,
    public BranchFinalizePolicy,
    public BranchReleasePolicy,
    public MetaDataNodeRetrievePolicy,
    public DataNodeRetrievePolicy,
    public DataStorageFlushPolicy,
    public ObsoletePolicy,
    public SyncPolicy,
    public DataClearPolicy,
    public OutOfCorePrefetchPolicy,
    public InCorePrefetchPolicy
{

    public:
        TensorControllerTemplate()
            : TensorController()
        {
        }

        void clear(TensorBlock* block)
        {
            DataClearPolicy::clear(block);
        }

        void validate(TensorBlock* block)
        {
            ValidatePolicy::validate(block);
        }

        void retrieve(
            TensorBlock* block
        )
        {
            TensorBranch* branch = block->get_branch();
            branch->set_parent(block);
            MempoolRetrievePolicy
                ::retrieve(block);
            MetaDataBranchRetrievePolicy
                ::retrieve(block);
            TensorDataControllerRetrievePolicy
                ::retrieve(block);
            DataBranchRetrievePolicy
                ::retrieve(block);
        }

        void renew(
            TensorBlock* block
        )
        {
            BranchRenewPolicy::renew(block);
        }

        void flush(
            TensorBlock* block
        )
        {
            BranchFlushPolicy
                ::flush(block);
        }
        
        void finalize(
            TensorBlock* block
        )
        {
            BranchFinalizePolicy
                ::finalize(block);
        }

        void release(
            TensorBlock* block
        )
        {
            BranchReleasePolicy
                ::release(block);
        }

        void retrieve(MetaDataNode* mdnode)
        {
            MetaDataNodeRetrievePolicy
                ::retrieve(mdnode);
        }

        bool need_recompute_data()
        {
            return DataNodeRetrievePolicy::need_recompute();
        }

        void retrieve(
            DataNode* dnode,
            const MetaDataNode* parent,
            const uli* indices
        )
        {
            DataNodeRetrievePolicy
                ::retrieve(dnode, parent, indices);
        }

        void flush_data(TensorBlock* block)
        {
            DataStorageFlushPolicy
                ::flush(block);
        }

        void obsolete(TensorBlock* block)
        {
            ObsoletePolicy
                ::obsolete(block);
        }

        void sync(TensorBlock* block)
        {
            SyncPolicy::sync(block);
        }

        void out_of_core_prefetch(TensorBlock* block)
        {
            OutOfCorePrefetchPolicy::prefetch(block);
        }

        void in_core_prefetch(TensorBlock* current_block, TensorBlock* prev_block)
        {
            InCorePrefetchPolicy::prefetch(current_block, prev_block);
        }

};


}

#endif // TENSORIMPL_H
