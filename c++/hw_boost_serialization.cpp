/*boost serialization hello world*/

#include <vector>
#include <string>
#include <iostream>
#include <sstream>


#include "boost/serialization/serialization.hpp"
#include "boost/archive/binary_iarchive.hpp"
#include "boost/archive/binary_oarchive.hpp"
#include <boost/serialization/export.hpp>
#include <boost/serialization/version.hpp>
//for std::vector
#include <boost/serialization/vector.hpp>

using namespace std;

class CMyData
{
private:
    
    /*
     *intrusive way
     */
    friend class boost::serialization::access;
    
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & _tag;
        ar & _text;
    }
    
    
public:
    CMyData():_tag(0), _text(""){}
    
    CMyData(int tag, std::string text):_tag(tag), _text(text){}
    
    int get_tag() const {return _tag;}
    std::string get_text() const {return _text;}
    
private:
    int _tag;
    std::string _text;
};


class CMyData_Child: public CMyData
{
private:
    
    /*
     *intrusive way with parent
     */
    
    friend class boost::serialization::access;
    
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar & boost::serialization::base_object<CMyData>(*this);
        ar & _number;
    }
    
    
public:
    CMyData_Child():_number(0.0){}
    
    CMyData_Child(int tag, std::string text, float number):CMyData(tag, text), _number(number){}
    
    float get_number() const{return _number;}
    
private:
    float _number;
};

class CMyData2
{
public:
    CMyData2():_tag(0), _text(""){}
    
    CMyData2(int tag, std::string text):_tag(tag), _text(text){}
    
    virtual ~CMyData2() = default;
    
    int _tag;
    std::string _text;
};

namespace boost {
namespace serialization {
    template<class Archive>
    /*
     *non_instrusive way
     */
    void serialize(Archive & ar, CMyData2 & d, const unsigned int version)
    {
        ar & d._tag;
        ar & d._text;
    }
}
}


class CMyData2_Child: public CMyData2
{
public:
    CMyData2_Child():_number(0.0){}
    
    CMyData2_Child(int tag, std::string text, float number):CMyData2(tag, text), _number(number){}
    
    virtual ~CMyData2_Child() = default;
    
    float _number;
};

namespace boost {
namespace serialization {
        
    template<class Archive>
    void serialize(Archive & ar, CMyData2_Child & d, const unsigned int version)
    {
        // serialize base class information
        ar & boost::serialization::base_object<CMyData2>(d);
        ar & d._number;
    }
    
}
}

//for child pointer extract from parent point(dynamic_cast)
BOOST_CLASS_EXPORT_GUID(CMyData2_Child, "CMyData_Child2")

class CMyDataContainerSTL{
public:
    std::vector<CMyData2*> vPointers;
};

namespace  boost {
namespace serialization{

    template <class Archive>
    void serialize(Archive & ar, CMyDataContainerSTL & d, const unsigned int version)
    {
        ar & d.vPointers;
    }
}
}

void archive_intrusive_test()
{
    CMyData d1(2012, "China, good luck");
    std::ostringstream os;
    boost::archive::binary_oarchive oa(os);
    oa & d1;//序列化到一个ostringstream里面
    
    std::string content = os.str();//content保存了序列化后的数据。
    
    CMyData d2;
    std::istringstream is(content);
    boost::archive::binary_iarchive ia(is);
    ia & d2;//从一个保存序列化数据的string里面反序列化，从而得到原来的对象。
    
    std::cout << "CMyData tag: " << d2.get_tag() << ", text: " << d2.get_text() << "\n";
}

void archive_intrusive_with_parent_test()
{
    CMyData_Child d1(2012, "China, good luck", 1.2);
    std::ostringstream os;
    boost::archive::binary_oarchive oa(os);
    oa & d1;//序列化到一个ostringstream里面
    
    std::string content = os.str();//content保存了序列化后的数据。
    
    CMyData_Child d2;
    std::istringstream is(content);
    boost::archive::binary_iarchive ia(is);
    ia & d2;//从一个保存序列化数据的string里面反序列化，从而得到原来的对象。
    
    std::cout << "CMyData_Child tag: " << d2.get_tag() << ", text: " << d2.get_text() << ", number: "<<d2.get_number() << "\n";
}

void archive_non_instrusive_test()
{
    CMyData2 d1(2012, "China, good luck");
    std::ostringstream os;
    boost::archive::binary_oarchive oa(os);
    oa & d1;//序列化到一个ostringstream里面
    
    std::string content = os.str();//content保存了序列化后的数据。
    
    CMyData2 d2;
    std::istringstream is(content);
    boost::archive::binary_iarchive ia(is);
    ia & d2;//从一个保存序列化数据的string里面反序列化，从而得到原来的对象。
    
    std::cout << "CMyData2 tag: " << d2._tag << ", text: " << d2._text << "\n";
}

void archive_non_instrusive_with_parent_test()
{
    CMyData2_Child d1(2012, "test non-intrusive child class", 5.6);
    std::ostringstream os;
    boost::archive::binary_oarchive oa(os);
    oa & d1;//序列化到一个ostringstream里面
    
    std::string content = os.str();//content保存了序列化后的数据。
    
    CMyData2_Child d2;
    std::istringstream is(content);
    boost::archive::binary_iarchive ia(is);
    ia & d2;//从一个保存序列化数据的string里面反序列化，从而得到原来的对象。
    
    std::cout << "CMyData2_Child tag: " << d2._tag << ", text: " << d2._text << ", number: "<<d2._number<<"\n";
}

void archive_non_instrusive_with_parent_stl_test()
{
    std::string content;
    {
        CMyData2 d1(1, "a");
        CMyData2_Child d2(2, "b", 1.5);
        
        CMyDataContainerSTL containter;
        containter.vPointers.resize(3);
        containter.vPointers[0] = &d1;
        containter.vPointers[1] = &d2;
        containter.vPointers[2] = &d1;
        
        std::ostringstream os;
        boost::archive::binary_oarchive oa(os);
        oa & containter;

        content = os.str();
    }
    
    {
        CMyDataContainerSTL containter;
        std::istringstream is(content);
        boost::archive::binary_iarchive ia(is);
        ia & containter;
        CMyData2* d1 = containter.vPointers[0];
        CMyData2_Child* d2 = dynamic_cast<CMyData2_Child*>(containter.vPointers[1]);
        CMyData2* d3 = containter.vPointers[2];
        cout << d1->_tag << " " <<d1->_text << endl;
        cout << d2->_tag << " " << d2->_text << " " << d2->_number << endl;
        cout << d3->_tag << " " << d3->_text << endl;
    }
    
}

/*
 *serialize 使用的是相同的序列化方式
 *save和load使用的是不同的序列化方式，防止在版本迭代的时候，添加数据或删减数据，造成的序列化错误
 */
class CMyData2Container
{
    friend class boost::serialization::access;
    std::vector<CMyData2 *> datas;
    std::string serial;
    template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        // note, version is always the latest when saving
        ar  & serial;
        ar  & datas;
    }
    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        if(version > 0)
            ar & serial;
        ar  & datas;
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER()
public:
    CMyData2Container(){}
};

BOOST_CLASS_VERSION(CMyData2Container, 1)

int main(int argc, char** argv)
{
    archive_intrusive_test();
    archive_intrusive_with_parent_test();
    archive_non_instrusive_test();
    archive_non_instrusive_with_parent_test();
    archive_non_instrusive_with_parent_stl_test();
}
