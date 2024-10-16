#ifndef KGNodeData_HH__
#define KGNodeData_HH__

namespace KGeoBag
{

/*
*
*@file KGNodeData.hh
*@class KGNodeData
*@brief
*@details
*
*<b>Revision History:<b>
*Date Name Brief Description
*Thu Apr  3 09:42:18 EDT 2014 J. Barrett (barrettj@mit.edu) First Version
*
*/


//class to hold very basic node data for tree reconstruction
class KGNodeData
{
  public:
    KGNodeData()
    {
        fID = -1;
        fChildIDs.resize(0);
    };
    virtual ~KGNodeData(){};

    unsigned int GetID() const
    {
        return fID;
    };
    void SetID(const unsigned int& id)
    {
        fID = id;
    };

    unsigned int GetNChildren() const
    {
        return fChildIDs.size();
    };
    unsigned int GetChildID(unsigned int index) const
    {
        return fChildIDs[index];
    };

    void GetChildIDs(std::vector<unsigned int>* child_ids) const
    {
        *child_ids = fChildIDs;
    };
    void SetChildIDs(const std::vector<unsigned int>* child_ids)
    {
        fChildIDs = *child_ids;
    };

  private:
    unsigned int fID;
    std::vector<unsigned int> fChildIDs;
};


template<typename Stream> Stream& operator>>(Stream& s, KGNodeData& aData)
{
    s.PreStreamInAction(aData);

    unsigned int id;
    s >> id;

    aData.SetID(id);

    unsigned int size;
    s >> size;

    std::vector<unsigned int> child_ids;
    child_ids.resize(size);

    for (unsigned int i = 0; i < size; i++) {
        unsigned int id;
        s >> id;
        child_ids[i] = id;
    }

    aData.SetChildIDs(&child_ids);

    s.PostStreamInAction(aData);
    return s;
}

template<typename Stream> Stream& operator<<(Stream& s, const KGNodeData& aData)
{
    s.PreStreamOutAction(aData);

    unsigned int id = aData.GetID();

    s << id;

    unsigned int n_children = aData.GetNChildren();
    s << n_children;

    for (unsigned int i = 0; i < n_children; i++) {
        unsigned int id = aData.GetChildID(i);
        s << id;
    }

    s.PostStreamOutAction(aData);

    return s;
}

}  // namespace KGeoBag


#endif /* KGNodeData_H__ */
