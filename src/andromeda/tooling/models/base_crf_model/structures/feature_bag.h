//-*-C++-*-

#ifndef ANDROMEDA_CRF_FEATURE_BAG_H
#define ANDROMEDA_CRF_FEATURE_BAG_H

namespace andromeda_crf
{
  namespace utils
  {
    class crf_feature_bag
    {
    public:

      typedef typename crf_feature::feature_body_type crf_feature_body_type;

      typedef std::map<crf_feature_body_type, int> map_type;

    public:

      crf_feature_bag();

      int Put(const crf_feature& i);

      int Id(const crf_feature& i) const;

      crf_feature Feature(int id) const;

      int Size() const;

      void Clear();

    private:

      map_type mef2id;
      std::vector<crf_feature> id2mef;
    };

    crf_feature_bag::crf_feature_bag()
    {}

    int crf_feature_bag::Put(const crf_feature& i)
    {
      auto itr = mef2id.find(i.body());
      
      if(itr == mef2id.end())
	{
	  const int id = id2mef.size();
	  id2mef.push_back(i);
	  mef2id[i.body()] = id;
	  return id;
	}
      
      return itr->second;
    }

    int crf_feature_bag::Id(const crf_feature& i) const
    {
      auto itr = mef2id.find(i.body());
      
      if(itr == mef2id.end())
	{
	  return -1;
	}
      
      return itr->second;
    }

    crf_feature crf_feature_bag::Feature(int id) const
    {
      //assert(id >= 0 && id < (int)id2mef.size());
      return id2mef.at(id);
    }

    int crf_feature_bag::Size() const
    {
      return id2mef.size();
    }

    void crf_feature_bag::Clear()
    {
      mef2id.clear();
      id2mef.clear();
    }
    
  }

}

#endif
