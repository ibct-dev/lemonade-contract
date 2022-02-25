import { Blockchain } from "@ibct-dev/sclest";
import { expect } from "chai";

function seconds_since_epoch(d: number) {
    return Math.floor(d / 1000);
}

beforeEach(async () => {
    await new Promise(resolve => setTimeout(resolve, 1000));
    console.log("----------------------");
});

describe("lemonade.c v2 컨트랙트 테스트", () => {
    const debug = 1;
    const isLocal = true;
    const bc = new Blockchain({ host: "127.0.0.1", port: "8888" }); // test

    /* 테스트 변수들 */
    const manager = "lemonade.c"; // 컨트랙트 배포 계정
    const user = "test.p";
    const anotherUser = "test1.p";
    let contractTester: any;
    let ledTokenTester: any;
    let ledTester: any;
    let errorCount = 0;
    const initialize = 1000000;
    const lemInitialize = 1000000;
    const stake = 10000;

    describe("테스트가 시작되면, 블록체인이 연결 되어야 함", async () => {
        it("블록체인을 연결함", async () => {
            expect(await bc.checkConntection()).eq(true);
        });
    });

    if (isLocal) {
        describe("로컬블록체인이라면, 컨트랙트를 새로 배포해야 함", async () => {
            it("배포 계정을 필요한 경우 생성하고, 컨트랙트를 배포함", async () => {
                try {
                    contractTester = await bc.createAccountByCreator(
                        manager,
                        "led"
                    );
                    await contractTester.setContract("lemonade.c");
                    expect(contractTester.accountName).eq(manager);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: account is already created: ${error}`
                    );
                    errorCount += 1;
                    contractTester = await bc.getAccount(manager);
                    expect(contractTester.accountName).eq(manager);
                }
            });
            it("테스트를 위한 추가 계정들을 생성함", async () => {
                try {
                    ledTokenTester = await bc.getAccount("led.token");
                    ledTester = await bc.getAccount("led");
                    await bc.createAccountByCreator(user, "led");
                    await bc.createAccountByCreator(anotherUser, "led");
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: account is already created: ${error}`
                    );
                    errorCount += 1;
                    ledTokenTester = await bc.getAccount("led.token");
                    ledTester = await bc.getAccount("led");
                    await bc.getAccount(user);
                    await bc.getAccount(anotherUser);
                }
            });
            it("led.code 권한 생성", async () => {
                try {
                    const authData = {
                        threshold: 1,
                        keys: [
                            {
                                key: 'EOS8EReqzz88PbvNa8afvTkAhAdfbwgfRwfy4AMwS3K2thvFaMD9S',
                                weight: 1
                            }
                        ],
                        accounts: [
                            {
                                permission:
                                {
                                    actor: manager,
                                    permission: "led.code"
                                },
                                weight: 1
                            }
                        ],
                        waits: [],
                    }
                    const actionResult = await ledTester.actions.updateauth(
                        {
                            account: manager,
                            permission: "active",
                            parent: "owner",
                            auth: authData,
                        },
                        [
                            {
                                actor: manager,
                                permission: "owner",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot transfer: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`${user} 유저가 사용할 토큰을 지급함`, async () => {
                try {
                    const actionResult = await ledTokenTester.actions.transfer(
                        {
                            from: "led",
                            to: user,
                            quantity: `${initialize}.0000 LED`,
                            memo: `SHOW ME THE MONEY`
                        },
                        [
                            {
                                actor: "led",
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot transfer: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`${anotherUser} 유저가 사용할 토큰을 지급함`, async () => {
                try {
                    const actionResult = await ledTokenTester.actions.transfer(
                        {
                            from: "led",
                            to: anotherUser,
                            quantity: `${initialize}.0000 LED`,
                            memo: `SHOW ME THE MONEY`
                        },
                        [
                            {
                                actor: "led",
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot transfer: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`${manager} 유저가 사용할 토큰을 지급함`, async () => {
                try {
                    const actionResult = await ledTokenTester.actions.transfer(
                        {
                            from: "led",
                            to: manager,
                            quantity: `${initialize}.0000 LED`,
                            memo: `SHOW ME THE MONEY`
                        },
                        [
                            {
                                actor: "led",
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot transfer: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`${manager} 유저가 사용할 거버넌스 토큰 생성`, async () => {
                try {
                    const actionResult = await ledTokenTester.actions.create(
                        {
                            issuer: manager,
                            maximum_supply: `${lemInitialize}.0000 LEM`,
                        },
                        [
                            {
                                actor: "led.token",
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot create: ${error}`
                    );
                    errorCount += 1;
                }
            });
        });
        it("led.code 권한 생성", async () => {
            try {
                const authData = {
                    threshold: 1,
                    keys: [
                        {
                            key: 'EOS8EReqzz88PbvNa8afvTkAhAdfbwgfRwfy4AMwS3K2thvFaMD9S',
                            weight: 1
                        }
                    ],
                    accounts: [
                        {
                            permission:
                            {
                                actor: manager,
                                permission: "led.code"
                            },
                            weight: 1
                        }
                    ],
                    waits: [],
                }
                const actionResult = await ledTester.actions.updateauth(
                    {
                        account: manager,
                        permission: "active",
                        parent: "owner",
                        auth: authData,
                    },
                    [
                        {
                            actor: manager,
                            permission: "owner",
                        },
                    ]
                );
                expect(actionResult).to.have.all.keys([
                    "transaction_id",
                    "processed",
                ]);
            } catch (error) {
                console.log(
                    `ERROR ${errorCount}: cannot transfer: ${error}`
                );
                errorCount += 1;
            }
        });

        describe("Initialize: init()", async () => {
            it(`${manager} 유저가 컨트랙트 세팅 초기화 실행`, async () => {
                try {
                    const actionResult = await contractTester.actions.init(
                        {},
                        [
                            {
                                actor: manager,
                                permission: "active",
                            },
                        ]
                    );
                    expect(actionResult).to.have.all.keys([
                        "transaction_id",
                        "processed",
                    ]);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: cannot initialize: ${error}`
                    );
                    errorCount += 1;
                }
            });
            it(`config 테이블에 생성 확인`, async () => {
                const tableResult = await contractTester.tables.configs();
                if (debug) console.log(`Configs\n${JSON.stringify(tableResult)}`);
                const onlyCreated = tableResult[0];
                expect(onlyCreated).to.deep.include({ id: 0 });
                expect(onlyCreated).to.deep.include({ is_active: 1 });
            });
        });
    }
});
