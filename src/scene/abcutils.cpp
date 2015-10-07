#include "abcutils.h"


static void accumXform( M44d &xf, IObject obj, chrono_t seconds )
{
    if ( IXform::matches( obj.getHeader() ) )
    {
        IXform x( obj, kWrapExisting );
        XformSample xs;
        ISampleSelector sel( seconds );
        x.getSchema().get( xs, sel );
        xf *= xs.getMatrix();
    }
}


M44d getFinalMatrix( IObject &iObj, chrono_t seconds )
{
    M44d xf;
    xf.makeIdentity();

    IObject parent = iObj.getParent();

    while ( parent )
    {
        accumXform( xf, parent, seconds );
        parent = parent.getParent();
    }

    return xf;
}


static void copy_props(Alembic::Abc::ICompoundProperty & iRead,
    Alembic::Abc::OCompoundProperty & iWrite, double seconds)
{
    std::size_t numChildren = iRead.getNumProperties();

    for (std::size_t i = 0; i < numChildren; ++i)
    {
        Alembic::AbcCoreAbstract::PropertyHeader header =
            iRead.getPropertyHeader(i);
        if (header.isArray())
        {
            Alembic::Abc::IArrayProperty inProp(iRead, header.getName());
            Alembic::Abc::OArrayProperty outProp(iWrite, header.getName(),
                header.getDataType(), header.getMetaData(),
                header.getTimeSampling());

            std::size_t numSamples = inProp.getNumSamples();

            Alembic::AbcCoreAbstract::ArraySamplePtr samp;
            Alembic::Abc::ISampleSelector sel(seconds);

            inProp.get(samp, sel);
            outProp.set(*samp);

        }
        else if (header.isScalar())
        {
            Alembic::Abc::IScalarProperty inProp(iRead, header.getName());
            std::size_t numSamples = inProp.getNumSamples();
            if (numSamples < 1) {
                std::cerr << "property has no samples skipping "  << header.getName() << " " << header.getDataType() << "\n";
                continue;
            }
            //std::cerr << numSamples << " "<< header.getName() << " " << header.getDataType() << "\n";
\
            Alembic::Abc::OScalarProperty outProp;
            try {
                outProp = Alembic::Abc::OScalarProperty(iWrite, header.getName(),
                    header.getDataType(), header.getMetaData(),
                    header.getTimeSampling());
            } catch (...) {
                std::cerr << "property already exits " << header.getName() << " " << header.getDataType() << "\n";

                continue;
            }


            std::vector<std::string> sampStrVec;
            std::vector<std::wstring> sampWStrVec;
            if (header.getDataType().getPod() ==
                Alembic::AbcCoreAbstract::kStringPOD)
            {
                sampStrVec.resize(header.getDataType().getExtent());
            }
            else if (header.getDataType().getPod() ==
                     Alembic::AbcCoreAbstract::kWstringPOD)
            {
                sampWStrVec.resize(header.getDataType().getExtent());
            }

            char samp[4096];


            Alembic::Abc::ISampleSelector sel(seconds);

            if (header.getDataType().getPod() ==
                Alembic::AbcCoreAbstract::kStringPOD)
            {
                inProp.get(&sampStrVec.front(), sel);
                outProp.set(&sampStrVec.front());
            }
            else if (header.getDataType().getPod() ==
                Alembic::AbcCoreAbstract::kWstringPOD)
            {
                inProp.get(&sampWStrVec.front(), sel);
                outProp.set(&sampWStrVec.front());
            }
            else
            {
                inProp.get(samp, sel);
                outProp.set(samp);
            }

        }
        else if (header.isCompound())
        {
            Alembic::Abc::OCompoundProperty outProp(iWrite,
                header.getName(), header.getMetaData());
            Alembic::Abc::ICompoundProperty inProp(iRead, header.getName());
            copy_props(inProp, outProp, seconds);
        }
    }
}

void copy_object(Alembic::Abc::IObject & iIn,
    Alembic::Abc::OObject & iOut, double seconds)
{
    std::size_t numChildren = iIn.getNumChildren();

    Alembic::Abc::ICompoundProperty inProps = iIn.getProperties();
    Alembic::Abc::OCompoundProperty outProps = iOut.getProperties();

    //std::cerr << iIn.getFullName() << "\n";

    copy_props(inProps, outProps, seconds);

    for (std::size_t i = 0; i < numChildren; ++i)
    {
        Alembic::Abc::IObject childIn(iIn.getChild(i));
        const AbcA::ObjectHeader & header = childIn.getHeader();

        if (ICamera::matches(header))
            std::cerr << "camera\n";

        Alembic::Abc::OObject childOut(iOut, childIn.getName(),
                                       childIn.getMetaData());

        copy_object(childIn, childOut, seconds);
    }
}

void export_abc(std::vector< IArchive > &archives, std::string path, double seconds)
{
    OArchive out_archive(Alembic::AbcCoreHDF5::WriteArchive(), path);
    OObject out_top = out_archive.getTop();
    for (int i = 0; i < archives.size(); i++) {
        IObject in_top = archives[i].getTop();

        copy_object(in_top, out_top, seconds);
    }
}
